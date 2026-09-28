# Industrial Logger

This workspace contains an STM32F407-based industrial logger generated from STM32CubeMX/IDE project files.

## Project layout

- `Core/` - generated application and initialization code
- `BSP/` - board support and peripheral helper code
- `Drivers/` - STM32 HAL and CMSIS libraries
- `Middlewares/` - FreeRTOS middleware
- `MDK-ARM/` - Keil MDK project files
- `Industrial_Logger.ioc` - CubeMX configuration source

## VS Code notes

The workspace is already configured for embedded development. The VS Code settings in `.vscode/settings.json` add the STM32 include paths and standard editor associations so the project can be indexed cleanly.

## Build toolchain

Use an Arm GNU toolchain or the STM32 VS Code extension toolchain configured on your machine to build this firmware.
