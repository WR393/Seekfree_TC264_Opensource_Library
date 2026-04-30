#include "zf_common_headfile.h"
#include "justfloat.h"

#pragma section all "cpu0_dsram"

// 前台电机限速任务周期，单位：ms。实时平衡环不在主循环中运行。
#define MAIN_MOTOR_LOOP_PERIOD_MS      (10U)

// 当前目标为零占空比，只保留斜坡和限速保护框架。
#define MAIN_MOTOR_TARGET_DUTY         (600)
#define MAIN_MOTOR_DUTY_STEP           (20)

// 电机转速保护阈值，单位：rpm。
#define MAIN_MOTOR_RPM_LIMIT           (650)
#define MAIN_MOTOR_RPM_HARD_LIMIT      (900)

// 车身倾倒保护阈值，单位：deg。
#define MAIN_MOTOR_STOP_PITCH_DEG      (35.0f)

static int16 main_abs_i16(int16 value)
{
    // 本文件内部使用的 int16 绝对值，避免引入额外库函数。
    return (value >= 0) ? value : (int16)(-value);
}

static float main_abs_f32(float value)
{
    // 本文件内部使用的 float 绝对值。
    return (value >= 0.0f) ? value : -value;
}

static void main_motor_limit_loop(void)
{
    // 主循环里的低频电机保护任务：读取转速和倾角，平滑更新占空比。
    static uint32 last_run_tick = 0U;
    static int16 current_duty = 0;
    int16 target_duty = MAIN_MOTOR_TARGET_DUTY;
    int16 abs_rpm = 0;
    float abs_pitch = 0.0f;

    if ((uwtick - last_run_tick) < MAIN_MOTOR_LOOP_PERIOD_MS)
    {
        // 未到周期直接返回，主循环仍可继续执行其他前台任务。
        return;
    }
    last_run_tick = uwtick;

    motor_get_speed();
    abs_rpm = main_abs_i16(motor_rpm);
    abs_pitch = main_abs_f32(pitch_balance);

    if ((abs_pitch > MAIN_MOTOR_STOP_PITCH_DEG) || (abs_rpm > MAIN_MOTOR_RPM_HARD_LIMIT))
    {
        // 大角度倾倒或严重超速时立即把目标占空比拉回 0。
        target_duty = 0;
    }
    else if (abs_rpm > MAIN_MOTOR_RPM_LIMIT)
    {
        target_duty = 0;
    }
    else if (abs_rpm > (MAIN_MOTOR_RPM_LIMIT - 100))
    {
        target_duty = MAIN_MOTOR_TARGET_DUTY / 2;
    }

    if (current_duty < target_duty)
    {
        // 斜坡限制占空比变化，避免电机命令阶跃。
        current_duty += MAIN_MOTOR_DUTY_STEP;
        if (current_duty > target_duty)
        {
            current_duty = target_duty;
        }
    }
    else if (current_duty > target_duty)
    {
        current_duty -= MAIN_MOTOR_DUTY_STEP;
        if (current_duty < target_duty)
        {
            current_duty = target_duty;
        }
    }

    motor_set_duty(current_duty);
}

int core0_main(void)
{
    // CPU0 负责板级初始化、应用层初始化、PIT 节拍和前台调度。
    clock_init();
    debug_init();
    justfloat_init();

    // 舵机 PWM 先初始化到中位，再做一次上电自检。
    pwm_init(ATOM1_CH1_P33_9, 330, mid);
    servo_startup_self_test();

    // 设备和应用层初始化顺序：电机 -> IMU -> 平衡控制 -> 前台调度。
    motor_init();
    system_delay_ms(500);
    imu_all_init();
    balance_init();
    scheduler_init();

    motor_set_duty(0);

    // PIT 周期分配：
    // 1ms  系统时基 uwtick
    // 2ms  IMU 更新 + 角速度内环
    // 10ms 角度环
    // 20ms 航向外环
    pit_ms_init(CCU60_CH0, 1);
    pit_ms_init(CCU60_CH1, 2);
    pit_ms_init(CCU61_CH0, 10);
    pit_ms_init(CCU61_CH1, 20);

    cpu_wait_event_ready();

    while (TRUE)
    {
        // 主循环只处理可延后的保护、日志和显示任务。
        main_motor_limit_loop();
        scheduler_run();
    }
}

#pragma section all restore
