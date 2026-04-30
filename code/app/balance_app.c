#include "balance_app.h"

// 航向目标每次最多平滑移动的角度，单位：deg。
// 只用于慢速方向环，避免目标航向突变直接传到平衡目标角。
#define HEADING_STEP (1.5f)

// 调试开关：置 1 时关闭航向保持，只验证角度环和角速度环。
#define BALANCE_DEBUG_DISABLE_HEADING (1)

// 接近直立且角速度很小时清掉角度环历史项，减少静止附近误差残留。
#define BALANCE_UPRIGHT_DEADBAND_DEG (0.3f)
#define BALANCE_GYRO_DEADBAND_DPS (3.0f)

// 上电零点采样条件：需要连续采到足够多的稳定姿态样本。
#define BALANCE_ZERO_SAMPLE_COUNT (100U)
#define BALANCE_ZERO_MAX_ATTEMPT_COUNT (2000U)
#define BALANCE_ZERO_MAX_GYRO_DPS (5.0f)
#define BALANCE_ZERO_MAX_PITCH_DEG (8.0f)

// 三级串级控制器：
// steering_pid：航向误差 -> 目标车身倾角
// angle_pid：车身倾角误差 -> 目标 pitch 角速度
// gyro_pid：pitch 角速度误差 -> 舵机 PWM 修正量
PID_T steering_pid;
PID_T angle_pid;
PID_T gyro_pid;

// 平衡控制链路的关键观测量，供串口日志和屏幕任务读取。
float balance_zero_angle = -1.4f;
float pitch_balance = 0.0f;
float target_yaw = 0.0f;
float target_yaw_smooth = 0.0f;
float target_angle = 0.0f;
float target_gyro_rate = 0.0f;
float servo_output = 0.0f;

static float balance_normalize_angle(float angle)
{
    // 航向角按 [-180, 180] 归一化，避免跨 0/360 度时误差突跳。
    while (angle > 180.0f)
    {
        angle -= 360.0f;
    }

    while (angle < -180.0f)
    {
        angle += 360.0f;
    }

    return angle;
}

void balance_init(void)
{
    float zero_sum = 0.0f;
    uint16 sample_count = 0U;
    uint16 attempt_count = 0U;

    // PID 输出限幅直接对应下一级目标或舵机修正量，调参时先确认执行周期。
    pid_init(&steering_pid, 0.2f, 0.0f, 0.2f, 0.0f, 15.0f);
    pid_init(&angle_pid,100.0f, 0.8f, 0.3f, 0.0f, 400.0f);
    pid_init(&gyro_pid, 2.0f, 0.0f, 0.0f, 0.0f, 1600.0f);
    pid_app_limit_integral(&angle_pid, -200.0f, 200.0f);

    servo_set(mid);
    motor_set_duty(0);

    // 静止上电时估计 pitch 零点；样本不稳定就重新累计。
    while ((sample_count < BALANCE_ZERO_SAMPLE_COUNT) &&
           (attempt_count < BALANCE_ZERO_MAX_ATTEMPT_COUNT))
    {
        attempt_count++;
        imu660rc_callback();
        imu_proc();

        if ((pitch > -BALANCE_ZERO_MAX_PITCH_DEG) &&
            (pitch < BALANCE_ZERO_MAX_PITCH_DEG) &&
            (gyro_y_rate > -BALANCE_ZERO_MAX_GYRO_DPS) &&
            (gyro_y_rate < BALANCE_ZERO_MAX_GYRO_DPS))
        {
            zero_sum += pitch;
            sample_count++;
        }
        else
        {
            zero_sum = 0.0f;
            sample_count = 0U;
        }

        system_delay_ms(2);
    }

    if (sample_count >= BALANCE_ZERO_SAMPLE_COUNT)
    {
        // 正常情况使用稳定样本平均值作为机械安装零点。
        balance_zero_angle = zero_sum / (float)BALANCE_ZERO_SAMPLE_COUNT;
    }
    else
    {
        // 超时后退化为当前姿态，保证初始化不会永久卡住。
        balance_zero_angle = pitch;
    }

    // 初始化结束时清掉控制历史，确保 PIT 第一次进入时从干净状态起算。
    pitch_balance = 0.0f;
    target_yaw = yaw;
    target_yaw_smooth = yaw;
    target_angle = 0.0f;
    target_gyro_rate = 0.0f;
    servo_output = 0.0f;
    pid_reset(&steering_pid);
    pid_reset(&angle_pid);
    pid_reset(&gyro_pid);
    servo_set(mid);
}

void balance_steering_loop(void)
{
    // 20ms 慢速外环：只负责方向保持，不直接驱动舵机。
#if BALANCE_DEBUG_DISABLE_HEADING
    target_angle = 0.0f;
#else
    float target_yaw_error = balance_normalize_angle(target_yaw - target_yaw_smooth);
    float yaw_error = 0.0f;

    if (target_yaw_error > HEADING_STEP)
    {
        // 目标航向分步逼近，限制外环给角度环的扰动。
        target_yaw_smooth = balance_normalize_angle(target_yaw_smooth + HEADING_STEP);
    }
    else if (target_yaw_error < -HEADING_STEP)
    {
        target_yaw_smooth = balance_normalize_angle(target_yaw_smooth - HEADING_STEP);
    }
    else
    {
        target_yaw_smooth = target_yaw;
    }

    yaw_error = balance_normalize_angle(yaw - target_yaw_smooth);
    target_angle = pid_calculate_by_error(&steering_pid, yaw_error);
#endif
}

void balance_angle_loop(void)
{
    // 10ms 中间环：用相对零点的 pitch 计算目标角速度。
    pitch_balance = pitch - balance_zero_angle;
    pid_set_target(&angle_pid, target_angle);

    if ((pitch_balance > -BALANCE_UPRIGHT_DEADBAND_DEG) &&
        (pitch_balance < BALANCE_UPRIGHT_DEADBAND_DEG) &&
        (gyro_y_rate > -BALANCE_GYRO_DEADBAND_DPS) &&
        (gyro_y_rate < BALANCE_GYRO_DEADBAND_DPS))
    {
        pid_reset(&angle_pid);
    }

    target_gyro_rate = pid_calculate_positional(&angle_pid, pitch_balance);

    // 角度环积分限幅在运行中重复约束，防止参数调试时积分边界漂移。
    pid_app_limit_integral(&angle_pid, -200.0f, 200.0f);
}

void balance_gyro_loop(void)
{
    int32 servo_duty = 0;

    // 2ms 内环：用 pitch 角速度闭环直接生成舵机 PWM 修正量。
    pid_set_target(&gyro_pid, target_gyro_rate);
    servo_output = pid_calculate_positional(&gyro_pid, gyro_y_rate);
    servo_duty = (int32)mid + (int32)servo_output;
    if (servo_duty < 0)
    {
        servo_duty = 0;
    }
    servo_set((uint32_t)servo_duty);
}

void pid_test(void)
{
    // 前台低优先级日志任务，100ms 打印一次，不放在 2ms/10ms 中断里。
    static uint32 last_print_tick = 0U;
    static uint8 header_printed = 0U;

    if ((uwtick - last_print_tick) >= 100U)
    {
        last_print_tick = uwtick;

        if (!header_printed)
        {
            header_printed = 1U;
            printf("LOG,t,pitch,zero,p,gy,ta,tg,so,pwm,rpm,md,ap,ai,ad,ao,gp,gi,gd,go\r\n");
        }

        printf("LOG,%lu,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%lu,%d,%d,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f\r\n",
               (unsigned long)uwtick,
               pitch,
               balance_zero_angle,
               pitch_balance,
               gyro_y_rate,
               target_angle,
               target_gyro_rate,
               servo_output,
               (unsigned long)servo_last_duty,
               motor_rpm,
               motor_last_duty,
               angle_pid.p_out,
               angle_pid.i_out,
               angle_pid.d_out,
               angle_pid.out,
               gyro_pid.p_out,
               gyro_pid.i_out,
               gyro_pid.d_out,
               gyro_pid.out);
    }
}
