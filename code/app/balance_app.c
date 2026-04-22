#include "balance_app.h"

#define BALANCE_SERVO_SIGN (-1.0f)
#define BALANCE_SERVO_DEFAULT_TRIM (-950)   // Final fixed center offset after mechanical centering
#define BALANCE_ANGLE_BIAS_DEG (0.0f)
#define BALANCE_STEERING_KP (0.20f)
#define BALANCE_STEERING_LIMIT_DEG (4.0f)
#define BALANCE_HEADING_ERROR_LIMIT_DEG (20.0f)
#define BALANCE_HEADING_DEADBAND_DEG (1.5f)

PID_T steering_pid;
PID_T angle_pid;
PID_T gyro_pid;

float target_angle = 0.0f;
float target_gyro_rate = 0.0f;
float servo_output = 0.0f;
float balance_heading_target_yaw = 0.0f;
uint8 balance_control_enabled = 0;

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
    pid_init(&steering_pid, BALANCE_STEERING_KP, 0.0f, 0.0f, 0.0f, BALANCE_STEERING_LIMIT_DEG);
    pid_init(&angle_pid, 10.0f, 0.0f, 0.0f, 0.0f, 200.0f);
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
    pid_reset(&steering_pid);
    pid_reset(&angle_pid);
    pid_reset(&gyro_pid);

    target_angle = 0.0f;
    target_gyro_rate = 0.0f;
    servo_output = 0.0f;
    balance_control_enabled = enabled;

    if (enabled)
    {
        balance_capture_heading_target();
    }
    else
    {
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

    heading_error = balance_wrap_angle(balance_heading_target_yaw - yaw);
    heading_error = balance_limit(heading_error, -BALANCE_HEADING_ERROR_LIMIT_DEG, BALANCE_HEADING_ERROR_LIMIT_DEG);

    if ((heading_error < BALANCE_HEADING_DEADBAND_DEG) && (heading_error > -BALANCE_HEADING_DEADBAND_DEG))
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
