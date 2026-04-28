#include "motor_app.h"

// 电机反馈和最近一次指令，供控制限速和调试日志读取。
int16_t motor_rpm = 0;
int16_t motor_last_duty = 0;
float wheel_speed = 0.0f;

void motor_init(void)
{
    // 当前无刷驱动通过 UART 协议控制，应用层只初始化封装接口。
    small_driver_uart_init();
}

void motor_set_duty(int16_t duty)
{
    // 占空比指令限幅，避免应用层把超范围命令直接传给驱动。
    if (duty > 5000)
    {
        duty = 5000;
    }
    if (duty < -5000)
    {
        duty = -5000;
    }

    motor_last_duty = duty;
    small_driver_set_duty((int16)duty, (int16)duty);
}

void motor_set_speed(int16_t speed)
{
    // 速度闭环目标限幅，单位：rpm。
    if (speed > 3000)
    {
        speed = 3000;
    }
    if (speed < -3000)
    {
        speed = -3000;
    }

    small_driver_set_speed((int16)speed, (int16)speed);
}

void motor_get_speed(void)
{
    // 主动刷新驱动反馈；按当前接线方向，左轮速度在应用层取反。
    small_driver_get_speed();
    motor_rpm = -motor_value.receive_left_speed_data;
    wheel_speed = motor_rpm * RPM_TO_WHEEL;
}

void motor_test(void)
{
    // 独立电机测试：占空比在正反方向间缓慢爬升。
    static int16_t duty = 0;
    static uint8_t duty_dir = 1;

    if (duty_dir)
    {
        duty += 100;
        if (duty >= 2000)
        {
            duty_dir = 0;
        }
    }
    else
    {
        duty -= 100;
        if (duty <= -2000)
        {
            duty_dir = 1;
        }
    }

    motor_set_duty(duty);
}
