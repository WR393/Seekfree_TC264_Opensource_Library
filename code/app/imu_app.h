/*
 * imu_app.h
 *
 * 平衡控制里会直接用到的全局姿态角和角速度。
 */

#ifndef CODE_IMU_H_
#define CODE_IMU_H_

#include "zf_common_headfile.h"

// 平衡控制直接读取的姿态量，角度单位 deg，角速度单位 deg/s。
extern float roll;
extern float pitch;
extern float yaw;
extern float gyro_x_rate;
extern float gyro_y_rate;

// 2ms 周期调用：刷新 pitch/yaw/gyro_*_rate。
void imu_proc(void);

// 上电初始化 IMU，并完成 yaw 和陀螺零偏校准。
void imu_all_init(void);

// 前台调试输出。
void imu_test(void);

#endif /* CODE_IMU_H_ */
