/*
 * motor_app.h
 *
 * 无刷电机控制与速度反馈封装。
 */

#ifndef CODE_APP_MOTOR_APP_H_
#define CODE_APP_MOTOR_APP_H_

#include "zf_common_headfile.h"

#define WHEEL_DIAMETER 0.064f

// RPM 转线速度，单位 m/s。
#define RPM_TO_WHEEL (WHEEL_DIAMETER * PI / 60)

extern int16_t motor_rpm;
extern float wheel_speed;

void motor_init(void);
void motor_set_duty(int16_t duty);
void motor_set_speed(int16_t speed);
void motor_get_speed(void);
void motor_test(void);

#endif /* CODE_APP_MOTOR_APP_H_ */
