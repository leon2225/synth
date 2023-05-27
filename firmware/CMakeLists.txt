# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/leon/Programs/pico/pico-sdk")

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(Synth C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


# Add executable. Default name is the project name, version 0.1
set(SOURCE_FILES main.cpp DAC.cpp Tone.cpp ToneSheduler.cpp)
add_executable(Synth ${SOURCE_FILES} )

pico_set_program_name(Synth "Synth")
pico_set_program_version(Synth "0.1")

# pico_enable_stdio_uart(Synth 0)
pico_enable_stdio_usb(Synth 1)

# Add pio 
pico_generate_pio_header(Synth ${CMAKE_CURRENT_LIST_DIR}/i2s.pio)

# Add the standard library to the build
target_link_libraries(Synth
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(Synth PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(Synth 
        hardware_pwm
        hardware_spi
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_clocks
        )

pico_add_extra_outputs(Synth)

