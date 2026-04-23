/*
 * imu_app.h
 *
 * 平衡控制里会直接用到的全局姿态角和角速度。
 */

#ifndef CODE_IMU_H_
#define CODE_IMU_H_

#include "zf_common_headfile.h"

extern float roll;
extern float pitch;
extern float yaw;
extern float gyro_x_rate;
extern float gyro_y_rate;

void imu_proc(void);
void imu_all_init(void);
void imu_test(void);

#endif /* CODE_IMU_H_ */
