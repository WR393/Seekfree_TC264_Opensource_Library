/*
 * screen_app.h
 *
 * IPS200 低优先级状态显示接口。
 */

#ifndef CODE_APP_SCREEN_APP_H_
#define CODE_APP_SCREEN_APP_H_

#include "zf_common_headfile.h"

// 屏幕硬件和默认颜色配置。
#define SCREEN_TYPE      (IPS200_TYPE_SPI)
#define SCREEN_DIR       (IPS200_PORTAIT)
#define SCREEN_PEN_COLOR (RGB565_WHITE)
#define SCREEN_BG_COLOR  (RGB565_BLACK)

void screen_init(void);
void screen_task(void);

#endif /* CODE_APP_SCREEN_APP_H_ */
