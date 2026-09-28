# Industrial Logger

基于 STM32F407 + FreeRTOS 的工业模拟信号采集与数据记录系统。

## 功能

- 500Hz ADC 采集（TIM 触发 + DMA 循环搬运）
- FreeRTOS 多任务架构（采集 / 处理 / LCD / SD 日志 / 按键 / 电源监测）
- LCD 波形显示 + 系统状态页（栈水位、CPU 占用率、SD 容量）
- SD 卡 FATFS 故障日志（含事故前 50 个采样点）
- 电源电压监测与告警
- RTC 时间戳记录

## 硬件

- MCU: STM32F407ZGT6（168MHz，192KB SRAM）
- LCD: HX8357D（3.5寸，480×320，FSMC 接口）
- SD 卡: SDIO + FATFS
- 调试器: DAP-Link

## 开发环境

- VSCode + CMake + GCC (arm-none-eabi)
- STM32CubeMX 生成初始化代码
- OpenOCD + DAP-Link 调试

## 目录说明

- **BSP/** — 板级支持包，由本人独立编写。
  - **BSP/drivers/** — 各外设驱动
  - **BSP/middleware/** — 通用算法与数据结构（环形缓冲区、滑动平均滤波器）
  - **BSP/board/** — 板级初始化（`board.c`），作为驱动层与应用层的统一入口，在 `main.c` 中调用
  - **BSP/task_app.c** — FreeRTOS 任务主体
- **Core/** — CubeMX 生成的应用与初始化代码
- **Drivers/** — STM32 HAL 库与 CMSIS
- **FATFS/** — FatFs 文件系统
- **Middlewares/** — FreeRTOS 中间件
- **MDK-ARM/** — Keil MDK 工程文件
- **cmake/** — CMake 构建脚本
- **Industrial_Logger.ioc** — CubeMX 配置源文件

## 编译

```bash
cmake -B build/Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build/Debug

