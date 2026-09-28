cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Industrial_Logger)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

    # ==================== BSP 用户代码 ====================
# BSP 头文件路径
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/BSP
    ${CMAKE_SOURCE_DIR}/BSP/APP
    ${CMAKE_SOURCE_DIR}/BSP/DRIVERS
    ${CMAKE_SOURCE_DIR}/BSP/MiddleWare
)

# BSP 源文件
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/BSP/board.c
    ${CMAKE_SOURCE_DIR}/BSP/task_app.c
    ${CMAKE_SOURCE_DIR}/BSP/DRIVERS/led.c
    ${CMAKE_SOURCE_DIR}/BSP/DRIVERS/key.c
    ${CMAKE_SOURCE_DIR}/BSP/DRIVERS/lcd.c
    ${CMAKE_SOURCE_DIR}/BSP/DRIVERS/sd_log.c
    ${CMAKE_SOURCE_DIR}/BSP/DRIVERS/adc_app.c
    ${CMAKE_SOURCE_DIR}/BSP/DRIVERS/power_monitor.c
    ${CMAKE_SOURCE_DIR}/BSP/DRIVERS/rtc_helper.c
    ${CMAKE_SOURCE_DIR}/BSP/MiddleWare/filter.c
    ${CMAKE_SOURCE_DIR}/BSP/MiddleWare/ring_buffer.c
)
