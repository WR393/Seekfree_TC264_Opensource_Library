/*
 * servo_app.c
 *
 *  Created on: 2025年11月11日
 *      Author:
 */

#include "servo_app.h"

/* 舵机占空比设置 */
void servo_set(uint32_t duty)
{
    if(duty <= l_max)
        duty = l_max;
    if(duty >= r_max)
        duty = r_max;
    pwm_set_duty(ATOM1_CH1_P33_9, duty); // ATOM1_CH1_P33_9 非固定引脚
}

/* 舵机测试 用于寻找中值*/
void servo_test(void)
{
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
//    printf("duty:%d\r\n",servo_motor_duty);
}
