/*
 * servo_app.h
 *
 * 转向舵机极限和基础控制接口。
 */

#ifndef CODE_APP_SERVO_APP_H_
#define CODE_APP_SERVO_APP_H_

#include "zf_common_headfile.h"

// 舵机 PWM 实测边界：l_max 左极限，mid 中位，r_max 右极限。
#define l_max 3320
#define mid   4620
#define r_max 6820

// 最近一次写入的舵机 PWM，用于调试日志观察。
extern uint32 servo_last_duty;

void servo_set(uint32_t duty);
void servo_test(void);
void servo_startup_self_test(void);

#endif /* CODE_APP_SERVO_APP_H_ */
