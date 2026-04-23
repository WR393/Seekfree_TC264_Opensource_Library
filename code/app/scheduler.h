/*
 * scheduler.h
 *
 * 模块测试用的软件调度器头文件。
 */

#ifndef CODE_APP_SCHEDULER_H_
#define CODE_APP_SCHEDULER_H_

#include "zf_common_headfile.h"

extern uint32_t uwtick;

void scheduler_init(void);
void scheduler_run(void);

#endif /* CODE_APP_SCHEDULER_H_ */
