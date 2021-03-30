cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()


add_executable(sbi-weather
    src/sbi-weather.c
    src/dht11.c
    src/i2c.c
    src/cli.c
    src/wipe.c
    src/info.c
    src/store.c
)

#
# Turn on -Wall and -Werror
#
# DEFECT: I can't keep these out of the tiny-usb code, I have
#         no idea why and it contains a 'strict-overflow' warning,
#         hence the -Wno-error=strict-overflow
#
target_compile_options( sbi-weather PRIVATE -Wall -Werror -Wno-error=strict-overflow )
  
target_link_libraries(sbi-weather
  pico_stdlib
  pico_multicore
  pico_unique_id
  hardware_flash
  hardware_adc
  hardware_i2c
  )

# enable usb output, disable uart output
pico_enable_stdio_usb(sbi-weather 1)
pico_enable_stdio_uart(sbi-weather 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(sbi-weather)
