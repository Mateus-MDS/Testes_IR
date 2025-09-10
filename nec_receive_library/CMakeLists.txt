add_library(nec_receive_library STATIC
    nec_receive.c
    nec_receive.h
)

# Gera o header da PIO
pico_generate_pio_header(nec_receive_library ${CMAKE_CURRENT_LIST_DIR}/nec_receive.pio)

target_link_libraries(nec_receive_library
    pico_stdlib
    hardware_pio
)

# Adiciona os includes (diretório atual + diretório binário onde o PIO gera cabeçalhos)
target_include_directories(nec_receive_library PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
