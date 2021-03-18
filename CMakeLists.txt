cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()


add_executable(sbi-weather
    src/sbi-weather.c
    src/cli.c
    src/wipe.c
    src/info.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(sbi-weather
  pico_stdlib
  pico_multicore
  hardware_flash
  hardware_adc
  )

# enable usb output, disable uart output
pico_enable_stdio_usb(sbi-weather 1)
pico_enable_stdio_uart(sbi-weather 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(sbi-weather)
