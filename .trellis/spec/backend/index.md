# 固件 Backend 规范

> 在这个仓库里，Trellis 的 `backend` 对应的是 TC264 固件运行时，而不是 Web 服务。

---

## 适用范围

本目录用于约束以下代码层：

- `user/`：启动入口、核初始化、前台主循环、中断入口
- `code/app/`：业务编排、姿态处理、控制环、低优先级调度任务
- `code/driver/`：本项目自定义驱动包装、协议封装、算法模块
- `libraries/`：逐飞公共库、底层外设驱动、设备驱动、Infineon SDK

当前代码的核心运行模型是：

- `user/cpu0_main.c` 负责启动顺序与前台循环
- `user/isr.c` 负责 1/2/10/20ms 节拍与外设中断分发
- `code/app/balance_app.c` 负责三级串级 PID 控制
- `code/app/schedule.c` 只负责非关键、可延后的前台任务

---

## 规范索引

| 文档 | 说明 | 状态 |
|------|------|------|
| [目录结构规范](./directory-structure.md) | 模块应该放在哪一层、初始化与执行链路怎么分层 | Filled |
| [运行时状态与持久化规范](./database-guidelines.md) | 全局变量归属、协议状态、Flash 持久化约束 | Filled |
| [错误处理规范](./error-handling.md) | 限幅、重试、丢包恢复、中断安全约束 | Filled |
| [日志与观测规范](./logging-guidelines.md) | `debug_init()`、`printf()`、串口观测与时序约束 | Filled |
| [质量规范](./quality-guidelines.md) | 命名、分层、评审清单、板级验证要求 | Filled |

---

## 开发前检查

修改固件代码前，至少按下面顺序阅读：

- 总是先读 [目录结构规范](./directory-structure.md)
- 总是再读 [质量规范](./quality-guidelines.md)
- 如果改动 `user/isr.c`、PIT 周期、PID、回调函数、控制环，补读 [错误处理规范](./error-handling.md)
- 如果新增 `printf()`、调试串口输出、上位机遥测、虚拟示波器，补读 [日志与观测规范](./logging-guidelines.md)
- 如果新增全局状态、协议缓存、标定量、Flash 配置，补读 [运行时状态与持久化规范](./database-guidelines.md)
- 总是浏览一次 [共享思考指引](../guides/index.md)

---

## 锚点文件

生成或修改代码时，优先对齐这些真实样例：

- `user/cpu0_main.c`：初始化顺序、前台 `while(TRUE)` 主循环
- `user/cpu1_main.c`：辅核入口模板
- `user/isr.c`：PIT / EXTI / UART / DMA 中断分发
- `code/app/balance_app.c`：三级控制环与全局控制状态
- `code/app/imu_app.c`：IMU 初始化、姿态转换、上位机发送
- `code/app/motor_app.c`：驱动包装层、限幅与物理量换算
- `code/app/schedule.c`：低优先级前台调度
- `code/driver/pid/pid_driver.c`：算法模块与输出限幅
- `code/driver/motor/small_driver_uart_control.c`：7 字节串口协议与解析恢复
- `libraries/zf_common/zf_common_headfile.h`：头文件统一入口

---

## 说明

- 当前仓库没有真实的 `frontend` 代码，`frontend` 规范暂不作为主要入口。
- 如果未来新增上位机、调参工具或图形界面，应单独建立对应 package，避免把 PC 侧逻辑混入固件 `backend` 规范。
- 当前 bootstrap 任务只覆盖现有固件仓库的真实约束，不预写未来 host-side / GUI / frontend 规范。
- 如无明确任务要求，默认沿用当前控制语义：平衡控制链路以 `pitch` 为主，`roll` 仅保留在 IMU / 遥测等现有路径。
