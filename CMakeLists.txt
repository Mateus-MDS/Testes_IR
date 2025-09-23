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
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

# Incluir Pico SDK
include(pico_sdk_import.cmake)

project(emissor C CXX ASM)

# Inicializar SDK
pico_sdk_init()

# Executável principal
add_executable(emissor
    emissor.c
)

# Configurar nome e versão
pico_set_program_name(emissor "emissor")
pico_set_program_version(emissor "1.0")

# Habilitar USB para debug
pico_enable_stdio_usb(emissor 1)
pico_enable_stdio_uart(emissor 0)


# Linkar bibliotecas necessárias
target_link_libraries(emissor
    pico_stdlib
    hardware_gpio
    hardware_timer
)

# Incluir diretórios
target_include_directories(emissor PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Gerar arquivos UF2
pico_add_extra_outputs(emissor)