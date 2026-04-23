/*
 * schedule.c
 *
 * 简单的软件调度器，主要用于模块单独测试。
 * 当前平衡工程主控制还是以 PIT 中断调度为主。
 */

#include "scheduler.h"

uint8_t task_num;
uint32_t uwtick = 0;

typedef struct
{
    void (*task_func)(void);
    uint32_t rate_ms;
    uint32_t last_run;
} task_t;

// 联调时按需打开要测试的任务。
static task_t scheduler_task[] =
{
    {servo_test, 10, 0},
    // {motor_test, 100, 0},
    {imu_test, 10, 0},
    // {motor_get_speed, 10, 0},
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

        if (now_time >= scheduler_task[i].rate_ms + scheduler_task[i].last_run)
        {
            scheduler_task[i].last_run = now_time;
            scheduler_task[i].task_func();
        }
    }
}
