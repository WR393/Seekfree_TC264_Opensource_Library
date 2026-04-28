/*
 * balance_app.h
 *
 * 单轨车平衡串级控制接口：
 * 航向环 -> 角度环 -> 角速度环 -> 舵机输出。
 */

#ifndef CODE_APP_BALANCE_APP_H_
#define CODE_APP_BALANCE_APP_H_

#include "zf_common_headfile.h"

extern PID_T steering_pid;
extern PID_T angle_pid;
extern PID_T gyro_pid;

// 平衡零点和控制中间量，主要用于调试日志、屏幕显示和跨环节观测。
extern float balance_zero_angle;
extern float pitch_balance;
extern float target_yaw;
extern float target_yaw_smooth;
extern float target_angle;
extern float target_gyro_rate;
extern float servo_output;

// 初始化 PID、估计 pitch 零点，并让舵机/电机进入安全初始状态。
void balance_init(void);

// 20ms 航向外环：输出 target_angle。
void balance_steering_loop(void);

// 10ms 角度环：输出 target_gyro_rate。
void balance_angle_loop(void);

// 2ms 角速度内环：输出舵机 PWM 修正。
void balance_gyro_loop(void);

// 前台调试日志任务，不参与实时控制。
void pid_test(void);

#endif /* CODE_APP_BALANCE_APP_H_ */
