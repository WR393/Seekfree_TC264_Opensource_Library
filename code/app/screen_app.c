/*
 * screen_app.c
 *
 * IPS200 状态显示任务。
 * 当前只显示平衡调试关键量，不在屏幕任务里修改控制参数。
 */

#include "screen_app.h"
#include "imu_app.h"
#include "motor_app.h"
#include "balance_app.h"
#include "remote_control_app.h"

void screen_init(void)
{
    // 屏幕属于低优先级观察设备，初始化失败不影响中断控制链路。
    ips200_set_dir(SCREEN_DIR);
    ips200_set_color(SCREEN_PEN_COLOR, SCREEN_BG_COLOR);
    ips200_init(SCREEN_TYPE);
    ips200_clear();
}

void screen_task(void)
{
    // 固定坐标刷新关键姿态、目标和执行量，便于板上快速判断控制状态。
    ips200_set_color(RGB565_CYAN, RGB565_BLACK);
    ips200_show_string(0, 0, "BALANCE DIRECT");

    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_show_string(0, 24, "P:");
    ips200_show_float(24, 24, pitch, 3, 2);
    ips200_show_string(112, 24, "Y:");
    ips200_show_float(144, 24, yaw, 3, 2);

    ips200_show_string(0, 48, "GY:");
    ips200_show_float(32, 48, gyro_y_rate, 4, 1);
    ips200_show_string(112, 48, "RPM:");
    ips200_show_int(152, 48, motor_rpm, 6);

    ips200_show_string(0, 72, "TY:");
    ips200_show_float(32, 72, target_yaw, 3, 2);
    ips200_show_string(112, 72, "TYS:");
    ips200_show_float(152, 72, target_yaw_smooth, 3, 2);

    ips200_show_string(0, 96, "TA:");
    ips200_show_float(32, 96, target_angle, 3, 2);
    ips200_show_string(112, 96, "TG:");
    ips200_show_float(144, 96, target_gyro_rate, 4, 1);

    ips200_show_string(0, 120, "SV:");
    ips200_show_float(32, 120, servo_output, 4, 1);
    ips200_show_string(112, 120, "RC:");
    ips200_show_uint(144, 120, remote_control_connected(), 1);
}
