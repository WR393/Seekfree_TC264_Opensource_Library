/*
 * imu_app.c
 *
 * IMU 应用层：
 * 1. 初始化 IMU660RC
 * 2. 上电时把 yaw 归零
 * 3. 把驱动层数据转换成全局姿态角和角速度
 */

#include "imu_app.h"

float roll = 0;
float pitch = 0;
float yaw = 0;
float gyro_x_rate = 0;
float gyro_y_rate = 0;

// 上电瞬间的 yaw 被视为 0 度航向。
static float yaw_offset = 0;

void usart_send(float roll, float pitch, float yaw, uint8_t fusion_sta)
{
    uint8_t buffer[15];
    uint8_t sumcheck = 0;
    uint8_t addcheck = 0;
    uint8_t index = 0;

    // 上位机协议里，欧拉角需要先放大 100 倍再转成 int16。
    int16_t roll_int = (int16_t)(roll * 100.0f);
    int16_t pitch_int = (int16_t)(pitch * 100.0f);
    int16_t yaw_int = (int16_t)(yaw * 100.0f);

    buffer[index++] = 0xAB;
    buffer[index++] = 0xDC;
    buffer[index++] = 0xFE;
    buffer[index++] = 0x03;
    buffer[index++] = 7;
    buffer[index++] = 0;

    buffer[index++] = (uint8_t)(roll_int & 0xFF);
    buffer[index++] = (uint8_t)((roll_int >> 8) & 0xFF);
    buffer[index++] = (uint8_t)(pitch_int & 0xFF);
    buffer[index++] = (uint8_t)((pitch_int >> 8) & 0xFF);
    buffer[index++] = (uint8_t)(yaw_int & 0xFF);
    buffer[index++] = (uint8_t)((yaw_int >> 8) & 0xFF);
    buffer[index++] = fusion_sta;

    for (int i = 0; i < index; i++)
    {
        sumcheck += buffer[i];
        addcheck += sumcheck;
    }

    buffer[index++] = sumcheck;
    buffer[index++] = addcheck;

    for (int i = 0; i < index; i++)
    {
        printf("%c", buffer[i]);
    }
}

void imu_all_init(void)
{
    // 初始化失败就一直重试，直到 IMU 正常工作。
    while (imu660rc_init(IMU660RC_QUARTERNION_480HZ))
    {
        system_delay_ms(100);
    }

    // 给 IMU 一点上电稳定时间。
    system_delay_ms(800);

    // 记录开机时的朝向，后面项目里把它当成 yaw = 0。
    yaw_offset = imu660rc_yaw;
    imu_proc();
}

void imu_proc(void)
{
    // 从驱动层取出姿态角。
    pitch = imu660rc_pitch;
    roll = imu660rc_roll;

    // 绝对 yaw -> 相对开机时刻的 yaw。
    yaw = imu660rc_yaw - yaw_offset;

    // 原始陀螺数据 -> 角速度 deg/s。
    gyro_x_rate = imu660rc_gyro_transition(imu660rc_gyro_x);
    gyro_y_rate = imu660rc_gyro_transition(imu660rc_gyro_y);

    // 把 yaw 约束到 [-180, 180]，方便做航向控制。
    if (yaw > 180)
    {
        yaw -= 360;
    }
    if (yaw < -180)
    {
        yaw += 360;
    }
}

void imu_test(void)
{
    // 最简单的 IMU 连通性测试打印。
    printf("%f,%f,%f\r\n", roll, pitch, yaw);
}
