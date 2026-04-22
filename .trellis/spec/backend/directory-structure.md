# 目录结构规范

> 固件目录按“入口层 / 应用层 / 驱动层 / 平台层”划分，不按 Web 项目的 controller/service/model 划分。

---

## 当前目录事实

```text
user/
|- cpu0_main.c        # 主核启动顺序与前台 while(TRUE)
|- cpu1_main.c        # 辅核模板入口
|- isr.c              # PIT / EXTI / UART / DMA 中断入口
`- *.h

code/
|- app/
|  |- balance_app.c   # 姿态控制编排
|  |- imu_app.c       # IMU 数据整形与上位机输出
|  |- motor_app.c     # 电机控制包装与速度换算
|  |- servo_app.c     # 舵机控制
|  |- schedule.c      # 前台协作式调度
|  `- *.h
`- driver/
   |- pid/            # 可复用控制算法
   `- motor/          # FOC 串口协议包装

libraries/
|- zf_common/         # 公共类型、调试、统一头文件入口
|- zf_driver/         # 底层 MCU 外设驱动
|- zf_device/         # 设备级驱动
|- zf_components/     # 组件与打印重定向
`- infineon_libraries/# 厂商 SDK / iLLD
```

---

## 分层职责

### `user/`

- 只放系统入口、核初始化、中断入口、最外层主循环。
- `user/cpu0_main.c` 负责初始化顺序，当前顺序是：
  `clock_init -> debug_init -> pwm_init -> motor_init -> imu_all_init -> balance_init -> scheduler_init -> pit_ms_init`
- `user/isr.c` 负责节拍和中断分发，不在这里堆业务判断。

### `code/app/`

- 放应用编排、控制链路、物理量换算、低优先级测试任务。
- 这一层可以依赖 `code/driver/` 和 `libraries/`，但不应自己拼串口协议或直接改寄存器。
- 当前的典型模块：
  `imu_app` 负责姿态值整形，
  `balance_app` 负责三级串级控制，
  `motor_app` 负责限幅和设备 API 包装，
  `schedule.c` 负责前台低优先级任务。

### `code/driver/`

- 放本项目自定义的“可复用算法”和“设备协议包装”。
- `pid_driver` 负责纯算法。
- `small_driver_uart_control` 负责 7 字节 FOC 串口协议，不应把这一层逻辑散落到 `motor_app` 或 ISR。

### `libraries/`

- 默认视为平台层和共享库。
- 优先从 `code/` 侧适配，不要把应用功能直接改进 `libraries/infineon_libraries/`。
- 只有当修改目标是“整个库对所有调用方都应改变”时，才考虑改库代码。

---

## 模块放置规则

- 新增启动或核级逻辑，放 `user/`。
- 新增周期控制、姿态计算、传感器融合、动作编排，放 `code/app/`。
- 新增通信协议包装、控制算法、设备抽象，放 `code/driver/`。
- 新增板级或芯片级底层接口，优先复用 `libraries/` 现有能力；必要时新增本地包装层，而不是把业务直接塞进库。

---

## 执行链路规则

当前代码有明确的执行链路，新增逻辑时必须先决定落在哪一段：

- 启动阶段：一次性初始化，放 `core0_main()`
- 高频实时阶段：由 PIT / 外设中断驱动，放 ISR 分发到 app/driver 回调
- 前台低优先级阶段：由 `scheduler_run()` 轮询执行，仅跑不影响控制稳定性的任务

明确约束：

- `balance_gyro_loop()`、`balance_angle_loop()`、`balance_steering_loop()` 运行在 2ms / 10ms / 20ms 中断节拍，不应改成前台轮询
- `scheduler_run()` 当前只适合 `servo_test()`、`imu_test()`、`motor_get_speed()` 这类可关闭的测试/观测任务
- `uwtick` 是前台调度的时间基准，但它由 1ms PIT ISR 维护

---

## 命名规范

- 文件名使用小写加模块语义后缀，如 `_app`、`_driver`、`isr`
- 头文件保护宏跟随路径，如 `CODE_APP_MOTOR_APP_H_`
- 对外函数使用模块前缀，如 `motor_set_speed`、`balance_init`、`pid_set_target`
- 全局变量必须表达物理语义或协议语义，如 `pitch`、`gyro_y_rate`、`motor_rpm`、`target_angle`

---

## 不要这样做

- 不要为了单个功能再新建顶层源代码目录，优先沿用 `user/`、`code/app/`、`code/driver/`
- 不要把校验、收包、组包逻辑写进 `code/app/`
- 不要把较长逻辑直接塞进 `user/isr.c`
- 不要让 `core0_main()` 变成“所有业务都写在一个文件里”的巨型入口
- 不要优先修改 `libraries/infineon_libraries/` 来实现应用功能

---

## 参考样例

- `user/cpu0_main.c`
- `user/isr.c`
- `code/app/balance_app.c`
- `code/app/imu_app.c`
- `code/app/motor_app.c`
- `code/app/schedule.c`
- `code/driver/pid/pid_driver.c`
- `code/driver/motor/small_driver_uart_control.c`

---

## AI Drift Guardrails

- 不要把新的闭环控制、姿态融合主链路、协议状态机搬进 `scheduler_run()`；前台调度只承载可延后、可关闭的任务。
- 不要把串口收包缓存、收包计数、校验中间量拆到 `code/app/`；这类状态继续留在 `code/driver/` 的协议 owner 内。
- 不要因为上位机/遥测存在 `roll` 输出，就把新的平衡控制链路切到 `roll`；当前控制路径仍以 `pitch` 为主语义。
- 新增 host-side、GUI、调参工具时，不在现有固件目录里硬塞“伪前端”结构，应新建独立 package。
