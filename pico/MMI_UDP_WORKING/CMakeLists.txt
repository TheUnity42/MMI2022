set(PROJECT  main)
cmake_minimum_required(VERSION 3.12)
set(PICO_BOARD pico_w)
include(../include/pico_sdk_import.cmake)
project(${PROJECT} C CXX)
pico_sdk_init()

add_executable(${PROJECT} ${PROJECT}.c)

target_sources(${PROJECT} PRIVATE
        ${PROJECT}.c
        )
target_compile_definitions(main PRIVATE
            WIFI_SSID=\"${WIFI_SSID}\"
            WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
            HOST=\"${HOST}\"
            )


target_include_directories(${PROJECT} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(${PROJECT}
     pico_cyw43_arch_lwip_threadsafe_background
     pico_stdlib hardware_adc pico_multicore
)

pico_add_extra_outputs(${PROJECT})
pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)