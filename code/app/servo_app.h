/*
 * servo_app.h
 *
 *  Created on: 2025年11月11日
 *      Author:
 */

#ifndef CODE_APP_SERVO_APP_H_
#define CODE_APP_SERVO_APP_H_

#include "zf_common_headfile.h"

/* 舵机宏定义需要自行测试 */
#define l_max  4220             // 左极限
#define mid    5520             // 中值
#define r_max  6820             // 右极限

void servo_set(uint32_t duty);
void servo_test(void);

#endif /* CODE_APP_SERVO_APP_H_ */
