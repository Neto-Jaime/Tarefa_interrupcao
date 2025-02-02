# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Definindo a placa
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Incluindo o SDK do Pico
include(pico_sdk_import.cmake)

# Definindo o nome do projeto
project(tarefa_interrupcao C CXX ASM)

# Inicializando o SDK do Raspberry Pi Pico
pico_sdk_init()

# Adicionando o executável
add_executable(tarefa_interrupcao tarefa_interrupcao.c)

# Criando diretório de arquivos gerados para PIO
file(MAKE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/generated)

# Gerando o header PIO
pico_generate_pio_header(tarefa_interrupcao ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio OUTPUT_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# Definindo as fontes do alvo
target_sources(tarefa_interrupcao PRIVATE tarefa_interrupcao.c)

# Linkando bibliotecas padrão e hardware
target_link_libraries(tarefa_interrupcao PRIVATE pico_stdlib hardware_pio hardware_uart)

# Habilitando a saída serial via UART
pico_enable_stdio_uart(tarefa_interrupcao 0)   # Habilitar UART
pico_enable_stdio_usb(tarefa_interrupcao 1)    # Desabilitar USB

# Adicionando saídas extras
pico_add_extra_outputs(tarefa_interrupcao)
