/*
 * servo_app.c
 *
 * 转向舵机应用层封装。
 */

#include "servo_app.h"

uint32 servo_last_duty = mid;

void servo_set(uint32_t duty)
{
    // 限制在实车已经测试过的左右极限内，保护舵机和转向机构。
    if (duty <= l_max)
    {
        duty = l_max;
    }
    else if (duty >= r_max)
    {
        duty = r_max;
    }

    servo_last_duty = duty;
    pwm_set_duty(ATOM1_CH1_P33_9, duty);
}

void servo_test(void)
{
    // 舵机扫动测试，用于确认左极限、中位和右极限。
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

void servo_startup_self_test(void)
{
    // 上电自检按“中-左-中-右-中”动作，便于肉眼确认舵机输出正常。
    servo_set(mid);
    system_delay_ms(300);
    servo_set(l_max);
    system_delay_ms(300);
    servo_set(mid);
    system_delay_ms(300);
    servo_set(r_max);
    system_delay_ms(300);
    servo_set(mid);
    system_delay_ms(300);
}
