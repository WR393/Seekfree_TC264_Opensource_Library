/*
 * remote_control_app.c
 *
 * 遥控启动门控应用层。
 * 读取 UART SBUS 接收驱动，输出“是否连接”和“是否允许启动”两个状态。
 */

#include "remote_control_app.h"

// CH2 油门触发作为启动/停止开关，通道值围绕 1024 摆动。
#define REMOTE_ENABLE_CHANNEL_INDEX      (1U)
#define REMOTE_TRIGGER_CENTER_VALUE      (1024U)

// 高低两个阈值形成迟滞，避免摇杆在临界点附近反复触发。
#define REMOTE_TRIGGER_ACTIVE_DELTA_HIGH (180U)
#define REMOTE_TRIGGER_ACTIVE_DELTA_LOW  (100U)

// 超过该时间没有新帧，认为遥控信号断开。
#define REMOTE_SIGNAL_TIMEOUT_MS         (120U)
#define REMOTE_CHANNEL_DEFAULT_VALUE     (REMOTE_TRIGGER_CENTER_VALUE)

// 遥控状态只在本模块维护，外部通过 getter 读取。
static uint16 remote_enable_channel_value = REMOTE_CHANNEL_DEFAULT_VALUE;
static uint32 remote_last_frame_tick_ms = 0U;
static uint8 remote_is_connected = 0U;
static uint8 remote_start_request = 0U;
static uint8 remote_trigger_pressed = 0U;

static void remote_control_update_start_request(void)
{
    uint16 trigger_offset = 0U;

    // 断联时强制撤销启动请求，防止车辆在失控状态下继续允许启动。
    if (!remote_is_connected)
    {
        remote_start_request = 0U;
        remote_trigger_pressed = 0U;
        return;
    }

    if (remote_enable_channel_value >= REMOTE_TRIGGER_CENTER_VALUE)
    {
        trigger_offset = remote_enable_channel_value - REMOTE_TRIGGER_CENTER_VALUE;
    }
    else
    {
        trigger_offset = REMOTE_TRIGGER_CENTER_VALUE - remote_enable_channel_value;
    }

    if (remote_trigger_pressed)
    {
        // 触发后必须回到低阈值以内，下一次拨动才会再次切换状态。
        if (trigger_offset < REMOTE_TRIGGER_ACTIVE_DELTA_LOW)
        {
            remote_trigger_pressed = 0U;
        }
    }
    else
    {
        if (trigger_offset > REMOTE_TRIGGER_ACTIVE_DELTA_HIGH)
        {
            // 每次有效拨动在启动允许/禁止之间翻转。
            remote_trigger_pressed = 1U;
            remote_start_request = remote_start_request ? 0U : 1U;
        }
    }
}

void remote_control_init(void)
{
    // 初始化为安全默认值：未连接、未请求启动。
    remote_enable_channel_value = REMOTE_CHANNEL_DEFAULT_VALUE;
    remote_last_frame_tick_ms = 0U;
    remote_is_connected = 0U;
    remote_start_request = 0U;
    remote_trigger_pressed = 0U;

    uart_receiver_init();
}

void remote_control_service(void)
{
    // 前台服务中消费接收完成标志，ISR/驱动只负责收包。
    if (uart_receiver.finsh_flag)
    {
        uart_receiver.finsh_flag = 0U;
        remote_enable_channel_value = uart_receiver.channel[REMOTE_ENABLE_CHANNEL_INDEX];
        remote_last_frame_tick_ms = uwtick;
        remote_is_connected = (1U == uart_receiver.state) ? 1U : 0U;
    }

    if (remote_is_connected)
    {
        // 用 1ms 系统时基判断帧间隔，超时后进入断联状态。
        if ((uwtick - remote_last_frame_tick_ms) > REMOTE_SIGNAL_TIMEOUT_MS)
        {
            remote_is_connected = 0U;
        }
    }

    remote_control_update_start_request();
}

uint8 remote_control_connected(void)
{
    return remote_is_connected;
}

uint8 remote_control_start_enabled(void)
{
    return remote_start_request;
}

uint16 remote_control_get_enable_channel_value(void)
{
    return remote_enable_channel_value;
}
