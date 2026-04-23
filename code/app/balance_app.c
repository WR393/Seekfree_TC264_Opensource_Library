#include "balance_app.h"

// 舵机方向符号。
// 如果车身向一侧倾倒时舵机纠偏方向反了，先检查这里。
#define BALANCE_SERVO_SIGN (-1.0f)

// 舵机机械回正后对应的软件中位补偿。
// 小车前轮“视觉上走直线”的位置，未必正好等于舵机 PWM 的理论中值。
#define BALANCE_SERVO_DEFAULT_TRIM (-950)

// 车身角度零点补偿。
// 一开始先保持 0，只有在舵机中位已经调准后，才考虑动这里。
#define BALANCE_ANGLE_BIAS_DEG (0.0f)

// 方向约束环只给一个很小的目标倾角。
// 它的作用是限制持续偏航，强度必须明显弱于平衡环。
#define BALANCE_STEERING_KP (0.20f)
#define BALANCE_STEERING_LIMIT_DEG (4.0f)
#define BALANCE_HEADING_ERROR_LIMIT_DEG (20.0f)
#define BALANCE_HEADING_DEADBAND_DEG (1.5f)

// 串级 PID 结构：
// 转向环 -> target_angle
// 角度环 -> target_gyro_rate
// 角速度环 -> servo_output
PID_T steering_pid;
PID_T angle_pid;
PID_T gyro_pid;

// 调试和观测用的中间量。
float target_angle = 0.0f;
float target_gyro_rate = 0.0f;
float servo_output = 0.0f;
float balance_heading_target_yaw = 0.0f;
uint8 balance_control_enabled = 0;

// 运行时舵机中位，可通过板载按键调整，不必重新编译。
static int32 balance_servo_trim = BALANCE_SERVO_DEFAULT_TRIM;

static float balance_wrap_angle(float angle)
{
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

static float balance_limit(float value, float min_value, float max_value)
{
    if (value < min_value)
    {
        return min_value;
    }

    if (value > max_value)
    {
        return max_value;
    }

    return value;
}

void balance_init(void)
{
    // 最外层：方向保持环，只做慢速纠偏。
    pid_init(&steering_pid, BALANCE_STEERING_KP, 0.0f, 0.0f, 0.0f, BALANCE_STEERING_LIMIT_DEG);

    // 中间层：角度环，把车身倾角误差变成目标角速度。
    pid_init(&angle_pid, 10.0f, 0.0f, 0.0f, 0.0f, 200.0f);

    // 最内层：角速度环，直接输出舵机控制量。
    pid_init(&gyro_pid, 7.0f, 0.0f, 0.0f, 0.0f, 1300.0f);
    pid_app_limit_integral(&angle_pid, -200.0f, 200.0f);

    target_angle = 0.0f;
    target_gyro_rate = 0.0f;
    servo_output = 0.0f;
    balance_heading_target_yaw = 0.0f;
    balance_control_enabled = 0;
    balance_apply_servo_center();
}

void balance_set_enabled(uint8 enabled)
{
    // 每次开关平衡控制时都清掉 PID 历史项，
    // 否则启动阶段会带着上一次的误差继续算。
    pid_reset(&steering_pid);
    pid_reset(&angle_pid);
    pid_reset(&gyro_pid);

    target_angle = 0.0f;
    target_gyro_rate = 0.0f;
    servo_output = 0.0f;
    balance_control_enabled = enabled;

    if (enabled)
    {
        // 平衡接管时，把当前航向记成目标航向。
        balance_capture_heading_target();
    }
    else
    {
        // 关闭平衡时，只保持舵机回到软件中位。
        balance_apply_servo_center();
    }
}

void balance_capture_heading_target(void)
{
    balance_heading_target_yaw = yaw;
    pid_reset(&steering_pid);
}

void balance_steering_loop(float steering_error)
{
    float heading_error = 0.0f;

    if (!balance_control_enabled)
    {
        target_angle = 0.0f;
        return;
    }

    // 航向保持故意做得很弱。
    // 它的目标是防止持续越跑越偏，不是替代平衡控制。
    heading_error = balance_wrap_angle(balance_heading_target_yaw - yaw);
    heading_error = balance_limit(heading_error,
                                  -BALANCE_HEADING_ERROR_LIMIT_DEG,
                                  BALANCE_HEADING_ERROR_LIMIT_DEG);

    if ((heading_error < BALANCE_HEADING_DEADBAND_DEG) &&
        (heading_error > -BALANCE_HEADING_DEADBAND_DEG))
    {
        heading_error = 0.0f;
    }

    target_angle = pid_calculate_positional(&steering_pid, heading_error + steering_error);
}

void balance_angle_loop(void)
{
    if (!balance_control_enabled)
    {
        target_gyro_rate = 0.0f;
        return;
    }

    // 车身倾角误差 -> 目标角速度。
    pid_set_target(&angle_pid, target_angle + BALANCE_ANGLE_BIAS_DEG);
    target_gyro_rate = pid_calculate_positional(&angle_pid, pitch);
}

void balance_gyro_loop(void)
{
    if (!balance_control_enabled)
    {
        servo_output = 0.0f;
        balance_apply_servo_center();
        return;
    }

    // 快速内环直接驱动舵机。
    pid_set_target(&gyro_pid, target_gyro_rate);
    servo_output = pid_calculate_positional(&gyro_pid, gyro_y_rate);
    servo_set((uint32_t)(mid + balance_servo_trim + (int32_t)(BALANCE_SERVO_SIGN * servo_output)));
}

void balance_set_servo_trim(int32 trim)
{
    balance_servo_trim = trim;
}

int32 balance_get_servo_trim(void)
{
    return balance_servo_trim;
}

void balance_apply_servo_center(void)
{
    servo_set((uint32_t)(mid + balance_servo_trim));
}
