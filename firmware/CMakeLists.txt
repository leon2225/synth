# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

set(SONG_NAME "sampleSong")

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
FILE(GLOB_RECURSE SOURCE_FILES "*.cpp" "*.c" "*.h")

# Remove build files from the list of source files
list(FILTER SOURCE_FILES EXCLUDE REGEX "build")

# Remove songs from the list of source files
list(FILTER SOURCE_FILES EXCLUDE REGEX "/songs/")
list(APPEND SOURCE_FILES "${CMAKE_SOURCE_DIR}/ui_songs/songs/${SONG_NAME}.cpp")

string (REGEX REPLACE "(^|[^\\\\]);" "\\1\n" SOURCE_FILES_MULTILINE "${SOURCE_FILES}")
message(STATUS "Source files: ${SOURCE_FILES_MULTILINE}")
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Print size of executable
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND arm-none-eabi-size ${CMAKE_CURRENT_LIST_DIR}/build/${PROJECT_NAME}.elf
    VERBATIM
)
string(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")


pico_set_program_name(${PROJECT_NAME} "Synth")
pico_set_program_version(${PROJECT_NAME} "0.1")

# pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

# Add pio 
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/i2s.pio)

# Add the standard library to the build
target_link_libraries(${PROJECT_NAME}
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(${PROJECT_NAME} 
        hardware_pwm
        hardware_spi
        hardware_dma
        hardware_pio
        hardware_interp
        hardware_timer
        hardware_clocks
        hardware_interp
        pico_multicore
        )

pico_add_extra_outputs(${PROJECT_NAME})

