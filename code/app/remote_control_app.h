/*
 * remote_control_app.h
 *
 * SBUS 遥控启动/停止门控接口。
 */

#ifndef CODE_APP_REMOTE_CONTROL_APP_H_
#define CODE_APP_REMOTE_CONTROL_APP_H_

#include "zf_common_headfile.h"

void remote_control_init(void);

// 前台周期调用：消费接收帧并更新连接/启动状态。
void remote_control_service(void);

// 当前是否收到有效遥控信号。
uint8 remote_control_connected(void);

// 当前是否允许车辆启动。
uint8 remote_control_start_enabled(void);

// 返回用于启动门控的原始通道值，便于屏幕或串口调试。
uint16 remote_control_get_enable_channel_value(void);

#endif /* CODE_APP_REMOTE_CONTROL_APP_H_ */
