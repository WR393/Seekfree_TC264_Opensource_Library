# 运行时状态与持久化规范

> 当前仓库没有数据库；这里的“database”对应固件里的运行时状态归属、协议缓存和未来的 Flash 持久化约束。

---

## 当前事实

本项目当前没有：

- 关系型数据库
- ORM
- migration

真正需要规范的是：

- 全局变量归谁维护
- 哪些状态只能在模块内部持有
- 哪些值可以通过 `extern` 暴露
- 如果以后落 Flash，页面布局和恢复策略怎么定义

---

## 状态归属规则

| 状态类型 | 当前归属 | 代码样例 | 规则 |
|----------|----------|----------|------|
| 前台调度时基 | `code/app/schedule.c` | `uwtick`, `task_num`, `scheduler_task[]` | 调度表内部状态尽量 `static`，只暴露 `uwtick` 和调度 API |
| 姿态状态 | `code/app/imu_app.c` | `roll`, `pitch`, `yaw`, `gyro_x_rate`, `gyro_y_rate` | IMU 原始值与整形值由 IMU 模块统一维护 |
| 控制状态 | `code/app/balance_app.c` | `steering_pid`, `angle_pid`, `gyro_pid`, `target_angle`, `target_gyro_rate`, `servo_output` | 控制环中间量由控制模块统一拥有 |
| 电机协议状态 | `code/driver/motor/small_driver_uart_control.c` | `motor_value` | 串口收发缓存、校验值、解析结果必须归协议模块 |
| 电机物理量 | `code/app/motor_app.c` | `motor_rpm`, `wheel_speed` | 对外暴露给应用层使用，但不要复制多份“真值” |

---

## 基本约束

- 优先让状态留在拥有它的模块内部。
- 只有跨模块确实需要共享时，才通过 `extern` 或小型 API 暴露。
- 协议缓存、收包计数、校验中间量优先保持在 `.c` 文件内部或专属结构体中。
- 一个模块只维护一份“权威状态”，不要让多个模块各自保存同一份语义相同的数据。

具体到当前代码：

- `scheduler_task[]` 是调度器内部表，保持 `static` 是正确做法。
- `motor_value` 把发包缓冲、收包缓冲、校验值和解析结果收敛到一个结构体，是推荐模式。
- `PID_T` 是通用结构体定义，但 `steering_pid` / `angle_pid` / `gyro_pid` 的生命周期属于 `balance_app`。

---

## 命名要求

- 变量名要体现物理意义或协议意义，不用 `temp`、`data1` 这类名字。
- 有单位含义时，优先让名字能体现单位或语义转换，如 `motor_rpm`、`wheel_speed`、`gyro_y_rate`。
- 协议缓存允许使用 `send_data_buffer`、`receive_data_buffer` 这类运输层命名，但必须放在协议所有者模块中。

---

## 场景：新增持久化参数或标定量

### 1. Scope / Trigger

- 触发条件：需要把参数在掉电后保留，例如 PID 标定值、零偏、设备配置

### 2. Signatures

新增持久化时，必须集中到单独模块，并至少提供清晰的读写接口，例如：

```c
void config_load(void);
void config_save(void);
void config_reset_default(void);
```

不要把 Flash 读写函数散落到 `imu_app.c`、`motor_app.c`、`balance_app.c` 里。

### 3. Contracts

- 必须先定义页号、字段顺序、字段类型、版本号、默认值
- 必须定义“首次上电 / 数据损坏 / 校验失败”时的恢复路径
- 必须明确谁负责触发写入，避免多个模块竞争写 Flash

### 4. Validation & Error Matrix

| 场景 | 必须行为 |
|------|----------|
| 第一次读取到空白页 | 加载默认值，不得直接使用未初始化数据 |
| 校验失败或版本不匹配 | 放弃旧数据，回退默认值，并保留可观测信号 |
| 高频控制路径请求写 Flash | 拒绝直接在 ISR 或高频路径写入，转移到安全时机 |

### 5. Good / Base / Bad Cases

- Good：单独 `config` 模块维护页面布局、默认值、CRC 和恢复逻辑
- Base：只有少量参数，也仍然通过统一模块管理，不在应用模块里直接写页
- Bad：`balance_app.c` 直接写 Flash 保存 PID，`imu_app.c` 再单独写零偏，页面布局无人维护

### 6. Tests Required

- 验证首次上电是否正确加载默认值
- 验证正常保存后掉电重启是否恢复
- 验证损坏数据或版本不匹配时是否回退默认值
- 验证高频控制环与写入路径不会互相阻塞

### 7. Wrong vs Correct

#### Wrong

- 在任意业务模块里随手调用底层 Flash API
- 没有默认值、版本号、校验和恢复路径

#### Correct

- 由单一配置模块统一封装 Flash 页布局和恢复逻辑
- 其他模块只读写结构化配置，不碰底层页面细节

---

## 常见错误

- 一个功能的状态分散在多个无关文件里，没有单一 owner
- 同一语义保存多份全局变量，后续不同步
- 直接把厂商库或设备驱动里的变量当成业务主状态
- 还没定义布局和恢复策略就开始写 Flash

---

## 参考样例

- `code/app/schedule.c`
- `code/app/imu_app.c`
- `code/app/motor_app.c`
- `code/app/balance_app.c`
- `code/driver/pid/pid_driver.h`
- `code/driver/motor/small_driver_uart_control.c`
- `libraries/zf_driver/zf_driver_flash.c`

---

## AI Drift Guardrails

- `roll`、`pitch`、`yaw` 同属 IMU 模块维护的姿态状态，但当前平衡控制真正消费的是 `pitch`；新增控制状态时应先复用这条语义边界，而不是复制一份新的姿态真值。
- 如果变量表达的是控制主语义，名字应与当前控制链路保持一致，例如继续使用 `pitch`、`target_angle`、`target_gyro_rate`，不要无故引入另一套同义姿态变量。
- 不要把 `motor_value.receive_data_count`、接收缓存或校验中间量暴露给 app 层直接操作。
- 不要因为 `imu_app.c` 里有 `roll` 遥测，就在新控制模块里再复制一套以 `roll` 为核心的闭环状态。
