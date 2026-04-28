/*
 * balance_tuning_app.c
 *
 * 运行时调参占位模块。
 * 当前平衡控制直接使用代码内参数，调参菜单暂时关闭，保留接口避免上层依赖断开。
 */

#include "balance_tuning_app.h"

void balance_tuning_init(void)
{
    // 调参菜单关闭时无需初始化状态。
}

void balance_tuning_service(void)
{
    // 保留低优先级服务入口，后续恢复按键/屏幕调参时复用。
}

void balance_tuning_log_task(void)
{
    // 调参日志关闭，避免干扰当前 PID 调试串口输出。
}

uint8 balance_tuning_get_menu_level(void)
{
    return BALANCE_TUNING_MENU_LOOP;
}

uint8 balance_tuning_get_selected_loop(void)
{
    return 0U;
}

uint8 balance_tuning_get_selected_param(void)
{
    return 0U;
}

const char *balance_tuning_get_menu_level_name(void)
{
    return "OFF";
}

const char *balance_tuning_get_loop_name(uint8 loop_index)
{
    (void)loop_index;
    return "OFF";
}

uint8 balance_tuning_get_param_count(uint8 loop_index)
{
    (void)loop_index;
    return 0U;
}

const char *balance_tuning_get_param_name(uint8 loop_index, uint8 param_index)
{
    (void)loop_index;
    (void)param_index;
    return "---";
}

float balance_tuning_get_value(uint8 loop_index, uint8 param_index)
{
    (void)loop_index;
    (void)param_index;
    return 0.0f;
}

uint8 balance_tuning_value_is_integer(uint8 loop_index, uint8 param_index)
{
    (void)loop_index;
    (void)param_index;
    return 0U;
}

float balance_tuning_get_selected_small_step(void)
{
    return 0.0f;
}

float balance_tuning_get_selected_large_step(void)
{
    return 0.0f;
}
