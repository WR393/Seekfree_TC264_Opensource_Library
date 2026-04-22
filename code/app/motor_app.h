/*
 * motor_app.h
 *
 *  Created on: 2025年11月12日
 *      Author:
 *
 */

#ifndef CODE_APP_MOTOR_APP_H_
#define CODE_APP_MOTOR_APP_H_

#include "zf_common_headfile.h"

#define WHEEL_DIAMETER              0.064f       // 轮子直径

#define RPM_TO_WHEEL                (WHEEL_DIAMETER * PI / 60)      // RPM 转 轮速公式

extern int16_t motor_rpm;
extern float wheel_speed;

void motor_init(void);              // 初始化（调用small_driver_uart_init）

void motor_set_duty(int16_t duty);       // 设置占空比（-10000~10000）

void motor_set_speed(int16_t speed);    // 设置速度

void motor_get_speed(void);                 // 获取速度

void motor_test(void);              // 电机正负转测试

#endif /* CODE_APP_MOTOR_APP_H_ */
