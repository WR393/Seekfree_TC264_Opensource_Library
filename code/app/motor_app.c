#include "motor_app.h"

// 最新的电机速度反馈。
int16_t motor_rpm = 0;
float wheel_speed = 0.0f;

void motor_init(void)
{
    // 当前无刷驱动通过串口通信。
    small_driver_uart_init();
}

void motor_set_duty(int16_t duty)
{
    // 先把占空比限制在驱动可接受的范围内。
    if (duty > 5000)
    {
        duty = 5000;
    }
    if (duty < -5000)
    {
        duty = -5000;
    }

    // 这个项目左右通道给同一个值。
    small_driver_set_duty((int16)duty, (int16)duty);
}

void motor_set_speed(int16_t speed)
{
    // 速度指令先限幅，再发给驱动。
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
    // 先主动刷新一次驱动返回的速度数据。
    small_driver_get_speed();

    // 按当前接线和定义，正向速度要在这里取反。
    motor_rpm = -motor_value.receive_left_speed_data;
    wheel_speed = motor_rpm * RPM_TO_WHEEL;
}

void motor_test(void)
{
    // 独立电机测试：占空比来回爬升，确认电机和驱动工作正常。
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
