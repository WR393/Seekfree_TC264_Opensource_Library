/*
 * balance_tuning_app.h
 *
 * 低优先级运行时调参菜单接口。
 */

#ifndef CODE_APP_BALANCE_TUNING_APP_H_
#define CODE_APP_BALANCE_TUNING_APP_H_

#include "zf_common_headfile.h"

#define BALANCE_TUNING_LOOP_COUNT  (4U)
#define BALANCE_TUNING_PARAM_COUNT (4U)

typedef enum
{
    // 菜单层级：先选控制环，再选参数，最后编辑参数值。
    BALANCE_TUNING_MENU_LOOP = 0,
    BALANCE_TUNING_MENU_PARAM,
    BALANCE_TUNING_MENU_EDIT,
} balance_tuning_menu_level_enum;

void balance_tuning_init(void);
void balance_tuning_service(void);
void balance_tuning_log_task(void);

uint8 balance_tuning_get_menu_level(void);
uint8 balance_tuning_get_selected_loop(void);
uint8 balance_tuning_get_selected_param(void);
const char *balance_tuning_get_menu_level_name(void);
const char *balance_tuning_get_loop_name(uint8 loop_index);
uint8 balance_tuning_get_param_count(uint8 loop_index);
const char *balance_tuning_get_param_name(uint8 loop_index, uint8 param_index);
float balance_tuning_get_value(uint8 loop_index, uint8 param_index);
uint8 balance_tuning_value_is_integer(uint8 loop_index, uint8 param_index);
float balance_tuning_get_selected_small_step(void);
float balance_tuning_get_selected_large_step(void);

#endif /* CODE_APP_BALANCE_TUNING_APP_H_ */
