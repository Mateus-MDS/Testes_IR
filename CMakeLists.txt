cmake_minimum_required(VERSION 3.13)

# Incluir Pico SDK
include(pico_sdk_import.cmake)

project(ir_web_controller C CXX ASM)

# Inicializar SDK
pico_sdk_init()

# Verificar se é Pico W
if (NOT TARGET pico_cyw43_arch_lwip_poll)
    message(FATAL_ERROR "Este projeto requer Raspberry Pi Pico W")
endif()

# Executável principal
add_executable(ir_web_controller
    ir_web_controller.c
)

# Configurar nome e versão
pico_set_program_name(ir_web_controller "IR Web Controller")
pico_set_program_version(ir_web_controller "1.0")

# Habilitar USB para debug
pico_enable_stdio_usb(ir_web_controller 1)
pico_enable_stdio_uart(ir_web_controller 0)

# Definir credenciais WiFi (substitua pelos seus)
target_compile_definitions(ir_web_controller PRIVATE
    WIFI_SSID=\"SUA_REDE_WIFI\"
    WIFI_PASSWORD=\"SUA_SENHA_WIFI\"
)

# Linkar bibliotecas necessárias
target_link_libraries(ir_web_controller
    pico_stdlib
    pico_cyw43_arch_lwip_poll
    hardware_gpio
    hardware_timer
)

# Incluir diretórios
target_include_directories(ir_web_controller PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Gerar arquivos UF2
pico_add_extra_outputs(ir_web_controller)