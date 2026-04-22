# 日志与观测规范

> 当前工程没有结构化日志系统。可观测性主要依赖 `debug_init()`、`printf()` 和逐飞库里的串口/示波器类工具，但所有观测都必须服从实时性。

---

## 当前观测手段

- `debug_init()` 初始化默认调试串口
- `printf()` 输出人可读调试信息
- `imu_app.c` 里的 `usart_send()` 发送打包后的姿态字节流
- 逐飞库里已有无线串口、虚拟示波器等辅助观测能力

---

## 推荐用法

- 启动阶段的一次性状态，用 `printf()` 或简单提示输出
- 低频调参信息，放在前台测试函数或低优先级调度任务里
- 高频波形、姿态、控制量，优先使用紧凑二进制帧或示波器通道，不要长文本刷屏
- 调试输出应可关闭，关闭后主功能仍然正确

当前代码样例：

- `user/cpu0_main.c` 在启动阶段调用 `debug_init()`
- `code/app/imu_app.c` 的 `imu_test()` 在前台打印 `roll,pitch,yaw`
- `code/app/imu_app.c` 的 `usart_send()` 使用字节流方式输出姿态帧
- `code/app/motor_app.c` 把调试打印保留在测试函数或注释状态，而不是常驻控制路径

---

## 严格禁止

- 在 1ms / 2ms / 10ms / 20ms 的 PIT ISR 内直接做大量 `printf()`
- 在每次前台循环都持续输出长文本
- 应用层和驱动层对同一事件重复打印
- 在还没确认时序余量前，把日志放进控制闭环路径

---

## 输出位置规则

| 位置 | 是否允许持续输出 | 说明 |
|------|------------------|------|
| `core0_main()` 初始化阶段 | 可以少量输出 | 适合打印一次性启动状态 |
| `scheduler_run()` 管理的测试任务 | 允许低频输出 | 适合姿态、速度、调参观测 |
| `user/isr.c` 高频中断 | 默认禁止 | 只保留必要回调和标志处理 |
| 协议驱动收发路径 | 谨慎 | 带宽和时序都更紧，必要时优先用二进制遥测 |

---

## 场景：新增遥测输出

### 1. Scope / Trigger

- 触发条件：需要把姿态、速度、PID 输出、传感器值送到上位机

### 2. Signatures

如果新增观测接口，必须先明确输出函数签名和调用频率，例如：

```c
void telemetry_send(void);
void usart_send(float roll, float pitch, float yaw, uint8_t fusion_sta);
```

### 3. Contracts

- 必须明确发送通道、帧格式、字节序、输出频率
- 必须明确该输出是在 ISR 里还是前台里调用
- 高频观测优先选择紧凑帧，不优先选择格式化文本

### 4. Validation & Error Matrix

| 情况 | 必须行为 |
|------|----------|
| 仅用于一次性 bring-up | 可以使用 `printf()` |
| 高频持续观测 | 应改用紧凑帧或示波器通道 |
| 控制环抖动变大 | 先停日志，再确认日志是否造成时序问题 |

### 5. Good / Base / Bad Cases

- Good：`imu_test()` 这种可开关、低频、前台输出
- Base：调试期短时间文本输出，但不进入中断和闭环
- Bad：2ms 中断内持续 `printf()` 控制量

### 6. Tests Required

- 打开日志后确认控制环和串口功能仍然稳定
- 关闭日志后确认功能行为不变
- 高频观测场景确认不会丢中断或明显拖慢主循环

### 7. Wrong vs Correct

#### Wrong

- 为了方便直接在 ISR 里输出完整文本日志

#### Correct

- 把文本输出放到测试任务，或者把高频量改成紧凑遥测帧

---

## 常见错误

- 调参完成后忘记关掉周期性打印
- ISR 和被 ISR 调用的函数里都打印同一信息
- 本来只需要观测几个字节，却使用大量格式化文本

---

## 参考样例

- `user/cpu0_main.c`
- `code/app/imu_app.c`
- `code/app/motor_app.c`
- `libraries/zf_common/zf_common_debug.c`
- `libraries/zf_device/zf_device_virtual_oscilloscope.c`

---

## AI Drift Guardrails

- 需要上位机观测时，优先复用现有的 `imu_test()`、`usart_send()` 一类前台/遥测路径，不要把临时观测直接挂到控制 ISR。
- 为了省事把 `printf()` 塞进 `imu_proc()`、`balance_gyro_loop()`、`balance_angle_loop()`、`balance_steering_loop()` 这类高频路径，视为违规。
- 把协议收包异常和控制调试混在同一条文本日志里，既难读，也会拉长时序。
- 如果只是为了验证当前平衡控制，应优先围绕 `pitch`、`gyro_y_rate`、`target_angle`、`servo_output` 这些现有控制量做观测，而不是临时改写控制语义。
