cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(mmitcp C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add pico_stdlib library which aggregates commonly used features
# target_link_libraries(main pico_stdlib hardware_adc)

add_executable(main
        picow_tcp_server.c
        tcp_server.c
        )
target_compile_definitions(main PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(main PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )
target_link_libraries(main
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        )

# enable usb output, disable uart output
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

pico_add_extra_outputs(main)       

