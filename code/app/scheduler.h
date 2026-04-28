/*
 * scheduler.h
 *
 * 模块测试用的软件调度器头文件。
 */

#ifndef CODE_APP_SCHEDULER_H_
#define CODE_APP_SCHEDULER_H_

#include "zf_common_headfile.h"

// 由 1ms PIT 维护的系统毫秒时基，前台调度器按它判断任务周期。
extern uint32_t uwtick;

// 初始化前台任务表。
void scheduler_init(void);

// 在主循环中反复调用，执行到期的低优先级任务。
void scheduler_run(void);

#endif /* CODE_APP_SCHEDULER_H_ */
