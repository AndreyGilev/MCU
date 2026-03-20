cmake_minimum_required(VERSION 3.15)

set(PICO_BOARD pico)
set(PICO_PLATFORM rp2040)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

include(pico_sdk_import.cmake)

project(05-display)
pico_sdk_init()

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main.c ${CMAKE_CURRENT_SOURCE_DIR}/stdio-task/stdio-task.c ${CMAKE_CURRENT_SOURCE_DIR}/protocol-task/protocol-task.c ${CMAKE_CURRENT_SOURCE_DIR}/led-task/led-task.c) 
add_subdirectory("../libs/ili9341-driver" "${PROJECT_BINARY_DIR}/ili9341")
target_link_libraries(${PROJECT_NAME} pico_stdlib ili9341 hardware_spi)

pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/memmap_rp2040.ld)

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

pico_add_extra_outputs(${PROJECT_NAME})
