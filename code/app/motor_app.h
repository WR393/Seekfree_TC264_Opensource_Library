/*
 * motor_app.h
 *
 * 无刷电机控制和速度反馈应用层封装。
 */

#ifndef CODE_APP_MOTOR_APP_H_
#define CODE_APP_MOTOR_APP_H_

#include "zf_common_headfile.h"

// 轮径单位：m；RPM_TO_WHEEL 输出单位：m/s。
#define WHEEL_DIAMETER (0.064f)
#define RPM_TO_WHEEL   (WHEEL_DIAMETER * PI / 60.0f)

// motor_rpm：驱动反馈转速，单位 rpm；wheel_speed：折算线速度，单位 m/s。
extern int16_t motor_rpm;
extern int16_t motor_last_duty;
extern float wheel_speed;

void motor_init(void);
void motor_set_duty(int16_t duty);
void motor_set_speed(int16_t speed);
void motor_get_speed(void);
void motor_test(void);

#endif /* CODE_APP_MOTOR_APP_H_ */
