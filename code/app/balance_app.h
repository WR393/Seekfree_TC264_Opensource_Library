/*
 * balance_app.h
 *
 * 单轨车平衡控制：
 * 转向环 -> 角度环 -> 角速度环 -> 舵机输出
 */

#ifndef CODE_APP_BALANCE_APP_H_
#define CODE_APP_BALANCE_APP_H_

#include "zf_common_headfile.h"

extern PID_T steering_pid;
extern PID_T angle_pid;
extern PID_T gyro_pid;

extern float target_angle;
extern float target_gyro_rate;
extern float servo_output;
extern float balance_heading_target_yaw;
extern uint8 balance_control_enabled;

void balance_init(void);
void balance_steering_loop(float steering_error);
void balance_angle_loop(void);
void balance_gyro_loop(void);

void balance_set_servo_trim(int32 trim);
int32 balance_get_servo_trim(void);
void balance_apply_servo_center(void);

// 开启或关闭整套平衡控制。
void balance_set_enabled(uint8 enabled);

// 把当前 yaw 记录成目标航向。
void balance_capture_heading_target(void);

#endif /* CODE_APP_BALANCE_APP_H_ */
