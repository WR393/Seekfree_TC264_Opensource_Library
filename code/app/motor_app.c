#include "motor_app.h"

int16_t motor_rpm = 0;
float wheel_speed = 0.0f;

void motor_init(void)
{
    small_driver_uart_init();
}

void motor_set_duty(int16_t duty)
{
    if (duty > 5000)
    {
        duty = 5000;
    }
    if (duty < -5000)
    {
        duty = -5000;
    }

    small_driver_set_duty((int16)duty, (int16)duty);
}

void motor_set_speed(int16_t speed)
{
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
    small_driver_get_speed();
    motor_rpm = -motor_value.receive_left_speed_data;
    wheel_speed = motor_rpm * RPM_TO_WHEEL;
}

void motor_test(void)
{
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
