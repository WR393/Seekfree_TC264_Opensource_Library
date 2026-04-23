#include "zf_common_headfile.h"

#pragma section all "cpu0_dsram"

// 当前平衡策略使用的固定巡航速度。
#define BALANCE_FIXED_SPEED_RPM 800

// 启动阶段先用开环占空比把电机拉起来，再切到闭环速度控制。
#define BALANCE_STARTUP_DUTY 1200
#define BALANCE_STARTUP_RPM 80

// 小车只有满足启动条件后，才允许平衡控制接管。
#define BALANCE_ENABLE_DELAY_MS 1200
#define BALANCE_ENABLE_MIN_RPM 300
#define BALANCE_ENABLE_MAX_PITCH_DEG 8.0f

// 调试和校准相关开关。
#define IMU_LOG_PERIOD_MS 20
#define IMU_FALL_STOP_PITCH_DEG 35.0f
#define ENABLE_SERVO_TRIM_CALIBRATION 0
#define SERVO_TRIM_RUNTIME_STEP 50
#define SERVO_TRIM_CALIBRATION_WINDOW_MS 3000
#define SERVO_TRIM_CALIBRATION_STEP_SMALL 10
#define SERVO_TRIM_CALIBRATION_STEP_LARGE 50

static void servo_trim_calibration(void)
{
    uint32 elapsed_ms = 0;
    int32 trim = balance_get_servo_trim();

    // 按键扫描周期设为 10 ms。
    key_init(10);
    key_clear_all_state();

    // 校准开始前，先让前轮回到软件中位。
    balance_apply_servo_center();

    printf("servo trim calibration start\r\n");
    printf("KEY1:-10  KEY1 long:-50  KEY2:+10  KEY2 long:+50\r\n");
    printf("KEY3 reset  KEY4 finish  current_trim:%ld\r\n", (long)trim);

    while (elapsed_ms < SERVO_TRIM_CALIBRATION_WINDOW_MS)
    {
        key_scanner();

        if (KEY_SHORT_PRESS == key_get_state(KEY_1))
        {
            trim -= SERVO_TRIM_CALIBRATION_STEP_SMALL;
            key_clear_state(KEY_1);
        }
        else if (KEY_LONG_PRESS == key_get_state(KEY_1))
        {
            trim -= SERVO_TRIM_CALIBRATION_STEP_LARGE;
            key_clear_state(KEY_1);
        }
        else if (KEY_SHORT_PRESS == key_get_state(KEY_2))
        {
            trim += SERVO_TRIM_CALIBRATION_STEP_SMALL;
            key_clear_state(KEY_2);
        }
        else if (KEY_LONG_PRESS == key_get_state(KEY_2))
        {
            trim += SERVO_TRIM_CALIBRATION_STEP_LARGE;
            key_clear_state(KEY_2);
        }
        else if ((KEY_SHORT_PRESS == key_get_state(KEY_3)) ||
                 (KEY_LONG_PRESS == key_get_state(KEY_3)))
        {
            trim = 0;
            key_clear_state(KEY_3);
        }
        else if ((KEY_SHORT_PRESS == key_get_state(KEY_4)) ||
                 (KEY_LONG_PRESS == key_get_state(KEY_4)))
        {
            key_clear_state(KEY_4);
            break;
        }
        else
        {
            system_delay_ms(10);
            elapsed_ms += 10;
            continue;
        }

        balance_set_servo_trim(trim);
        balance_apply_servo_center();
        printf("servo trim:%ld center_pwm:%ld\r\n", (long)trim, (long)(mid + trim));

        // 每次调完都重新开始计时，避免还没调完就自动退出。
        elapsed_ms = 0;
        system_delay_ms(10);
    }

    printf("servo trim calibration done: %ld\r\n", (long)balance_get_servo_trim());
}

static void handle_servo_trim_keys(void)
{
    int32 trim = balance_get_servo_trim();

    key_scanner();

    // 运行时中位微调：
    // 当小车“基本能平衡，但前轮还不够正”时，用这个更方便。
    if (KEY_SHORT_PRESS == key_get_state(KEY_1))
    {
        trim -= SERVO_TRIM_RUNTIME_STEP;
        balance_set_servo_trim(trim);
        key_clear_state(KEY_1);
        printf("servo trim:%ld\r\n", (long)trim);
    }
    else if (KEY_SHORT_PRESS == key_get_state(KEY_2))
    {
        trim += SERVO_TRIM_RUNTIME_STEP;
        balance_set_servo_trim(trim);
        key_clear_state(KEY_2);
        printf("servo trim:%ld\r\n", (long)trim);
    }
}

int core0_main(void)
{
    // 板级时钟和调试串口初始化。
    clock_init();
    debug_init();

    // 初始化舵机 PWM 输出。
    pwm_init(ATOM1_CH1_P33_9, 330, mid);

    // 先把平衡控制内部状态清零，再初始化电机和 IMU。
    balance_init();

#if ENABLE_SERVO_TRIM_CALIBRATION
    servo_trim_calibration();
#endif

    key_init(10);
    motor_init();
    imu_all_init();

    // 周期任务分配：
    // 1 ms  -> 系统时基
    // 2 ms  -> IMU 更新 + 角速度环
    // 10 ms -> 角度环
    // 20 ms -> 转向环
    pit_ms_init(CCU60_CH0, 1);
    pit_ms_init(CCU60_CH1, 2);
    pit_ms_init(CCU61_CH0, 10);
    pit_ms_init(CCU61_CH1, 20);

    cpu_wait_event_ready();

    while (TRUE)
    {
        int16 abs_rpm = (motor_rpm >= 0) ? motor_rpm : (int16)(-motor_rpm);
        float abs_pitch = (pitch >= 0.0f) ? pitch : -pitch;

        // 一旦平衡接管成功，本次上电周期内就一直保持使能。
        static uint8 balance_armed = 0;

        handle_servo_trim_keys();

        // 启动阶段：
        // 先强行拉起电机转动，再切到固定速度控制。
        if (abs_rpm < BALANCE_STARTUP_RPM)
        {
            motor_set_duty(BALANCE_STARTUP_DUTY);
        }
        else
        {
            motor_set_speed(BALANCE_FIXED_SPEED_RPM);
        }

        // 每次主循环都刷新一次速度反馈。
        motor_get_speed();

        // 平衡接管条件：
        // 1. 上电已经过了一段时间
        // 2. 电机速度已经起来
        // 3. 当前车身姿态没有大角度倾倒
        // 这样可以避免刚上电时舵机猛打。
        if (!balance_armed)
        {
            if ((uwtick >= BALANCE_ENABLE_DELAY_MS) &&
                (abs_rpm >= BALANCE_ENABLE_MIN_RPM) &&
                (abs_pitch <= BALANCE_ENABLE_MAX_PITCH_DEG))
            {
                balance_set_enabled(1);
                balance_armed = 1;
                printf("balance armed yaw:%0.3f trim:%ld\r\n",
                       yaw,
                       (long)balance_get_servo_trim());
            }
            else
            {
                balance_set_enabled(0);
            }
        }

        system_delay_ms(10);
    }
}

#pragma section all restore
