# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

include_directories( ${CMAKE_SOURCE_DIR}/Lib) # Inclui os files .h na pasta lib

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(emissor C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(nec_receive_library)
add_subdirectory(nec_transmit_library)

# Add executable. Default name is the project name, version 0.1

add_executable(emissor emissor.c Lib/ssd1306.c)

pico_set_program_name(emissor "emissor")
pico_set_program_version(emissor "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(emissor 0)
pico_enable_stdio_usb(emissor 1)

# Add the standard library to the build
target_link_libraries(emissor
        pico_stdlib
        hardware_pio
        hardware_clocks
        hardware_pwm
        hardware_i2c
        hardware_gpio
        hardware_irq
        hardware_sync
        hardware_flash
        nec_receive_library
        nec_transmit_library)

# Add the standard include files to the build
target_include_directories(emissor PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(emissor)

