/*
 * imu_app.c
 *
 * IMU 应用层：
 * 1. 初始化 IMU660RC
 * 2. 估计陀螺零偏和 yaw 零点
 * 3. 输出平衡控制直接使用的姿态角和角速度
 */

#include "imu_app.h"

float roll = 0.0f;
float pitch = 0.0f;
float yaw = 0.0f;
float gyro_x_rate = 0.0f;
float gyro_y_rate = 0.0f;

// 上电校准得到的零点，后续姿态量都按这些零点转换为相对值。
static float yaw_offset = 0.0f;
static float gyro_x_offset = 0.0f;
static float gyro_y_offset = 0.0f;

// pitch 使用陀螺积分和加速度倾角互补融合，减少单一传感器噪声。
static float pitch_filter = 0.0f;
static uint8 pitch_filter_ready = 0U;

// imu_proc() 当前由 2ms PIT 调用，周期单位为秒。
#define IMU_PROC_PERIOD_S          (0.002f)
#define IMU_PITCH_GYRO_WEIGHT      (0.98f)
#define IMU_PITCH_ACC_WEIGHT       (1.0f - IMU_PITCH_GYRO_WEIGHT)

static uint8 imu_value_is_valid(float value)
{
    // 简单过滤 NaN 和明显异常的大数，避免坏数据进入控制环。
    return ((value == value) && (value < 100000.0f) && (value > -100000.0f)) ? 1U : 0U;
}

static float imu_normalize_angle(float angle)
{
    // yaw 相对角限制在 [-180, 180]，便于方向环直接计算误差。
    while (angle > 180.0f)
    {
        angle -= 360.0f;
    }
    while (angle < -180.0f)
    {
        angle += 360.0f;
    }
    return angle;
}

static float imu_pitch_from_acc(void)
{
    // 根据加速度方向估计 pitch，输出单位：deg。
    float acc_x = imu660rc_acc_transition(imu660rc_acc_x);
    float acc_y = imu660rc_acc_transition(imu660rc_acc_y);
    float acc_z = imu660rc_acc_transition(imu660rc_acc_z);
    float horizontal = sqrtf(acc_y * acc_y + acc_z * acc_z);

    if (!imu_value_is_valid(acc_x) || !imu_value_is_valid(horizontal))
    {
        return pitch;
    }

    return atan2f(acc_x, horizontal) * 57.2957795f;
}

void usart_send(float roll, float pitch, float yaw, uint8_t fusion_sta)
{
    // 上位机欧拉角协议：角度放大 100 倍后按小端 int16 发送。
    uint8_t buffer[15];
    uint8_t sumcheck = 0;
    uint8_t addcheck = 0;
    uint8_t index = 0;
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
    float gyro_x_sum = 0.0f;
    float gyro_y_sum = 0.0f;
    uint16 i = 0U;

    // 初始化失败时持续重试，保证后续平衡控制不会在无 IMU 状态下启动。
    while (imu660rc_init(IMU660RC_QUARTERNION_480HZ))
    {
        system_delay_ms(100);
    }

    // 校准期间关闭 IMU 外部中断，避免采样过程被异步回调打断。
    exti_disable(IMU660RC_INT2_PIN);
    system_delay_ms(800);

    // 采集约 400ms 的静止陀螺数据，估计 X/Y 轴零偏。
    for (i = 0U; i < 200U; i++)
    {
        imu660rc_callback();
        gyro_x_sum += imu660rc_gyro_transition(imu660rc_gyro_x);
        gyro_y_sum += imu660rc_gyro_transition(imu660rc_gyro_y);
        system_delay_ms(2);
    }

    gyro_x_offset = gyro_x_sum / 200.0f;
    gyro_y_offset = gyro_y_sum / 200.0f;

    // 当前朝向记为 yaw = 0，后续 yaw 都是相对上电方向。
    if (imu_value_is_valid(imu660rc_yaw))
    {
        yaw_offset = imu660rc_yaw;
    }
    else
    {
        yaw_offset = 0.0f;
    }

    pitch_filter = imu_pitch_from_acc();
    pitch = pitch_filter;
    pitch_filter_ready = 1U;

    // 零点采样完成后再恢复 IMU 数据就绪中断。
    exti_enable(IMU660RC_INT2_PIN);

    imu_proc();
}

void imu_proc(void)
{
    float acc_pitch = 0.0f;

    // 驱动层原始陀螺值转换为 deg/s，并扣掉上电静止零偏。
    gyro_x_rate = imu660rc_gyro_transition(imu660rc_gyro_x) - gyro_x_offset;
    gyro_y_rate = imu660rc_gyro_transition(imu660rc_gyro_y) - gyro_y_offset;

    acc_pitch = imu_pitch_from_acc();
    if (!pitch_filter_ready)
    {
        pitch_filter = acc_pitch;
        pitch_filter_ready = 1U;
    }

    // 互补滤波：短时间信任陀螺响应，长时间用加速度修正漂移。
    pitch_filter = (IMU_PITCH_GYRO_WEIGHT * (pitch_filter + gyro_y_rate * IMU_PROC_PERIOD_S)) +
                   (IMU_PITCH_ACC_WEIGHT * acc_pitch);
    pitch = pitch_filter;

    if (imu_value_is_valid(imu660rc_roll))
    {
        roll = imu660rc_roll;
    }

    if (imu_value_is_valid(imu660rc_yaw) && imu_value_is_valid(yaw_offset))
    {
        yaw = imu_normalize_angle(imu660rc_yaw - yaw_offset);
    }
}

void imu_test(void)
{
    // 前台连通性测试输出，避免放进高频中断路径。
    printf("%f,%f,%f\r\n", roll, pitch, yaw);
}
