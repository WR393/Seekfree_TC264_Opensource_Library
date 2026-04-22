/*
 * imu.h
 *
 *  Created on: 2025年11月14日
 *      Author:
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
