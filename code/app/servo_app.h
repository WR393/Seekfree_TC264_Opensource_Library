/*
 * servo_app.h
 *
 * 转向舵机极限和基础控制接口。
 */

#ifndef CODE_APP_SERVO_APP_H_
#define CODE_APP_SERVO_APP_H_

#include "zf_common_headfile.h"

// 这三个值必须在实车上测出来。
#define l_max 4220
#define mid   5520
#define r_max 6820

void servo_set(uint32_t duty);
void servo_test(void);

#endif /* CODE_APP_SERVO_APP_H_ */
