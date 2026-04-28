/*
 * schedule.c
 *
 * 前台低优先级软件调度器。
 * 实时平衡控制在 PIT 中断中完成，这里只放日志、屏幕、测试等可延后任务。
 */

#include "scheduler.h"
#include "balance_app.h"
#include "motor_app.h"

uint8_t task_num;
uint32_t uwtick = 0;

typedef struct
{
    void (*task_func)(void);
    uint32_t rate_ms;      // 任务周期，单位：ms。
    uint32_t last_run;
} task_t;

// 联调时按需打开任务；不要把高频闭环控制放到前台调度器。
static task_t scheduler_task[] =
{
    {pid_test, 10, 0},
};

void scheduler_init(void)
{
    task_num = sizeof(scheduler_task) / sizeof(task_t);
}

void scheduler_run(void)
{
    for (uint8_t i = 0; i < task_num; i++)
    {
        uint32_t now_time = uwtick;

        // uwtick 由 1ms PIT 维护，前台只根据时间戳判断任务是否到期。
        if (now_time >= scheduler_task[i].rate_ms + scheduler_task[i].last_run)
        {
            scheduler_task[i].last_run = now_time;
            scheduler_task[i].task_func();
        }
    }
}
