#include "isr_config.h"
#include "isr.h"
#include "balance_app.h"

// 1ms 周期中断:
// 只维护系统基础时基, 供延时和简单调度使用.
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU60_CH0);
    uwtick++;
}

// 2ms 周期中断:
// 先刷新一次 IMU 姿态与角速度, 再执行平衡内环陀螺仪控制.
// 这一层是整车响应最快的一环, 负责先把车身的倾倒趋势压住.
IFX_INTERRUPT(cc60_pit_ch1_isr, 0, CCU6_0_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU60_CH1);
    imu_proc();
    balance_gyro_loop();
}

// 10ms 周期中断:
// 执行平衡角度环, 根据车身倾角修正目标控制量.
// 角度环速度比陀螺仪内环慢, 主要负责把车身拉回到目标姿态附近.
IFX_INTERRUPT(cc61_pit_ch0_isr, 0, CCU6_1_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU61_CH0);
    balance_angle_loop();
}

// 20ms 周期中断:
// 执行转向环, 当前默认目标方向给 0, 即尽量保持直行.
// 这一层最慢, 只负责方向修正, 不直接承担快速平衡任务.
IFX_INTERRUPT(cc61_pit_ch1_isr, 0, CCU6_1_CH1_ISR_PRIORITY)
{
    interrupt_global_enable(0);
    pit_clear_flag(CCU61_CH1);
    balance_steering_loop();
}

// 外部中断分组:
// 这里只是把对应通道的中断标志清掉, 当前没有挂应用层处理逻辑.
IFX_INTERRUPT(exti_ch0_ch4_isr, 0, EXTI_CH0_CH4_INT_PRIO)
{
    interrupt_global_enable(0);

    if (exti_flag_get(ERU_CH0_REQ0_P15_4))
    {
        exti_flag_clear(ERU_CH0_REQ0_P15_4);
    }

    if (exti_flag_get(ERU_CH4_REQ13_P15_5))
    {
        exti_flag_clear(ERU_CH4_REQ13_P15_5);
    }
}

// 外部中断分组:
// CH1 这一路接了 TOF 模块数据就绪信号, 中断里直接转发给测距模块处理.
// CH5 当前仍然只清标志位, 预留给后续外设扩展.
IFX_INTERRUPT(exti_ch1_ch5_isr, 0, EXTI_CH1_CH5_INT_PRIO)
{
    interrupt_global_enable(0);

    if (exti_flag_get(ERU_CH1_REQ10_P14_3))
    {
        exti_flag_clear(ERU_CH1_REQ10_P14_3);
        tof_module_exti_handler();
    }

    if (exti_flag_get(ERU_CH5_REQ1_P15_8))
    {
        exti_flag_clear(ERU_CH5_REQ1_P15_8);
    }
}

// 外部中断分组:
// CH3 是 IMU 数据就绪中断, 这里触发 imu660rc_callback() 读取新数据.
// 如果这一层没进来, 上层姿态解算就会一直拿不到新采样.
IFX_INTERRUPT(exti_ch3_ch7_isr, 0, EXTI_CH3_CH7_INT_PRIO)
{
    interrupt_global_enable(0);

    if (exti_flag_get(ERU_CH3_REQ3_P10_3))
    {
        exti_flag_clear(ERU_CH3_REQ3_P10_3);
        imu660rc_callback();
    }

    if (exti_flag_get(ERU_CH7_REQ16_P15_1))
    {
        exti_flag_clear(ERU_CH7_REQ16_P15_1);
    }
}

// 摄像头 DMA 传输完成中断:
// 把底层 DMA 完成事件转发给图像模块做收尾处理.
IFX_INTERRUPT(dma_ch5_isr, 0, DMA_INT_PRIO)
{
    interrupt_global_enable(0);
    camera_dma_handler();
}

// UART0 发送中断:
// 当前未额外挂接应用层逻辑, 仅保留中断入口.
IFX_INTERRUPT(uart0_tx_isr, 0, UART0_TX_INT_PRIO)
{
    interrupt_global_enable(0);
}

// UART0 接收中断:
// 若打开调试串口中断模式, 在这里把收到的数据交给调试口处理.
IFX_INTERRUPT(uart0_rx_isr, 0, UART0_RX_INT_PRIO)
{
    interrupt_global_enable(0);
#if DEBUG_UART_USE_INTERRUPT
    debug_interrupr_handler();
#endif
}

// UART1 发送中断:
// 当前未额外挂接应用层逻辑, 仅保留中断入口.
IFX_INTERRUPT(uart1_tx_isr, 0, UART1_TX_INT_PRIO)
{
    interrupt_global_enable(0);
}

// UART1 接收中断:
// 这里接的是无刷驱动控制串口, 收到返回帧后立即交给电机控制模块解析.
IFX_INTERRUPT(uart1_rx_isr, 0, UART1_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    uart_control_callback();
}

// UART2 发送中断:
// 当前未额外挂接应用层逻辑, 仅保留中断入口.
IFX_INTERRUPT(uart2_tx_isr, 0, UART2_TX_INT_PRIO)
{
    interrupt_global_enable(0);
}

// UART2 接收中断:
// 无线模块的串口接收入口, 数据到达后转发给无线通信模块处理.
IFX_INTERRUPT(uart2_rx_isr, 0, UART2_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    wireless_module_uart_handler();
}

// UART3 发送中断:
// 当前未额外挂接应用层逻辑, 仅保留中断入口.
IFX_INTERRUPT(uart3_tx_isr, 0, UART3_TX_INT_PRIO)
{
    interrupt_global_enable(0);
}

// UART3 接收中断:
// GNSS 模块的数据入口, 由导航模块在回调里解析定位信息.
IFX_INTERRUPT(uart3_rx_isr, 0, UART3_RX_INT_PRIO)
{
    interrupt_global_enable(0);
    gnss_uart_callback();
}

// UART0 异常中断:
// 统一交给底层 ASCLIN 驱动处理串口错误状态.
IFX_INTERRUPT(uart0_er_isr, 0, UART0_ER_INT_PRIO)
{
    interrupt_global_enable(0);
    IfxAsclin_Asc_isrError(&uart0_handle);
}

// UART1 异常中断:
// 统一交给底层 ASCLIN 驱动处理串口错误状态.
IFX_INTERRUPT(uart1_er_isr, 0, UART1_ER_INT_PRIO)
{
    interrupt_global_enable(0);
    IfxAsclin_Asc_isrError(&uart1_handle);
}

// UART2 异常中断:
// 统一交给底层 ASCLIN 驱动处理串口错误状态.
IFX_INTERRUPT(uart2_er_isr, 0, UART2_ER_INT_PRIO)
{
    interrupt_global_enable(0);
    IfxAsclin_Asc_isrError(&uart2_handle);
}

// UART3 异常中断:
// 统一交给底层 ASCLIN 驱动处理串口错误状态.
IFX_INTERRUPT(uart3_er_isr, 0, UART3_ER_INT_PRIO)
{
    interrupt_global_enable(0);
    IfxAsclin_Asc_isrError(&uart3_handle);
}
