cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(hellommi C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# # rest of your project
# add_executable(main
#     main.c
# )

# # Add pico_stdlib library which aggregates commonly used features
# target_link_libraries(main pico_stdlib)

# # enable usb output, disable uart output
#pico_enable_stdio_usb(main 1)
# pico_enable_stdio_uart(main 0)

# # create map/bin/hex/uf2 file in addition to ELF.
# pico_add_extra_outputs(main)

add_executable(picow_tcpip_server_background
        picow_tcp_server.c
        )
target_compile_definitions(picow_tcpip_server_background PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(picow_tcpip_server_background PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )
target_link_libraries(picow_tcpip_server_background
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        )
pico_add_extra_outputs(picow_tcpip_server_background)


pico_enable_stdio_usb(picow_tcpip_server_background 1)
pico_enable_stdio_uart(picow_tcpip_server_background 0)
add_executable(picow_tcpip_server_poll
        picow_tcp_server.c
        )
target_compile_definitions(picow_tcpip_server_poll PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        )
target_include_directories(picow_tcpip_server_poll PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )
target_link_libraries(picow_tcpip_server_poll
        pico_cyw43_arch_lwip_poll
        pico_stdlib
        )
pico_add_extra_outputs(picow_tcpip_server_poll)
