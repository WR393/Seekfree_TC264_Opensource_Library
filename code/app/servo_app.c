/*
 * servo_app.c
 *
 * 转向舵机控制封装。
 */

#include "servo_app.h"

void servo_set(uint32_t duty)
{
    // 限制在已经测试过的左右极限内，保护机械结构。
    if (duty <= l_max)
    {
        duty = l_max;
    }
    if (duty >= r_max)
    {
        duty = r_max;
    }

    pwm_set_duty(ATOM1_CH1_P33_9, duty);
}

void servo_test(void)
{
    // 慢速来回摆动，用于找左极限 / 中位 / 右极限。
    static uint16_t servo_motor_duty = mid;
    static uint8_t servo_motor_dir = 0;

    if (servo_motor_dir)
    {
        servo_motor_duty += 10;
        if (servo_motor_duty >= r_max)
        {
            servo_motor_dir = 0x00;
        }
    }
    else
    {
        servo_motor_duty -= 10;
        if (servo_motor_duty <= l_max)
        {
            servo_motor_dir = 0x01;
        }
    }

    servo_set(servo_motor_duty);
}
