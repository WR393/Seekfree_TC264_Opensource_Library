# Seekfree TC264 Opensource Library

基于逐飞 `TC264 Opensource Library` 的单轨车平衡控制工程。

当前工程主要用于：
- TC264 平台下的舵机 / 无刷电机 / IMU 联调
- 单车平衡控制实验
- 平衡、航向约束、遥控门控与实车调参验证

## 项目说明

这个仓库是在逐飞 TC264 开源库基础上做的实车控制工程，核心目标是通过 IMU 姿态反馈和舵机转向控制，让单轨车完成动态平衡。

当前代码中已经包含：
- 舵机控制
- 无刷电机占空比 / 速度反馈封装
- IMU660RC 姿态读取
- 串级 PID 平衡控制
- 遥控启动 / 停止门控
- IPS200 状态显示
- 平衡调试日志

## 目录结构

- `user/`
  - 主函数、中断服务、系统入口
- `code/app/`
  - 平衡、IMU、电机、舵机等应用层逻辑
- `code/driver/`
  - PID、电机驱动封装等
- `libraries/`
  - 逐飞库与 Infineon TC264 底层库

## 当前状态

目前工程已经完成以下阶段：
- 模块级测试通过：舵机、电机、IMU
- 已建立航向环、角度环、角速度环三级串级控制结构
- 已加入 IMU 零点估计、pitch 互补滤波和异常值保护
- 已加入舵机 PWM 限幅、电机转速保护和前台调试日志
- 已预留遥控门控、屏幕显示和运行时调参接口

仍在继续优化的部分：
- 落地发车稳定性
- 比赛场地橡胶跑道上的平衡成功率
- 航向保持与蛇形轨迹抑制
- 舵机摆动幅度与控制平顺性

## 开发环境

- MCU: Infineon TC264
- Library: Seekfree TC264 Opensource Library
- 工程类型: AURIX / ADS 工程

## 关键文件

- `user/cpu0_main.c`
  - 板级初始化、应用层初始化、PIT 启动与前台任务调度
- `user/isr.c`
  - IMU 与各控制环周期调用
- `code/app/balance_app.c`
  - 航向环、角度环、角速度环和平衡调试日志
- `code/app/imu_app.c`
  - 姿态角、角速度、yaw 零点和 pitch 滤波处理
- `code/app/motor_app.c`
  - 电机速度读取与控制
- `code/app/servo_app.c`
  - 舵机 PWM 输出
- `code/app/remote_control_app.c`
  - SBUS 遥控启动 / 停止门控
- `code/app/screen_app.c`
  - IPS200 运行状态显示
- `code/driver/pid/pid_driver.c`
  - PID 算法与限幅接口

## 备注

这个仓库目前以实车调试和控制验证为主，不是通用成品方案。PID 参数、舵机中位、启动策略、方向约束强度都需要结合具体车体结构和实际场地继续调整。
