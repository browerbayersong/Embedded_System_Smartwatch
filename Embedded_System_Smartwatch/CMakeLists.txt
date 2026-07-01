cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake" CACHE FILEPATH "Toolchain file")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

project(SmartwatchFirmware LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(SMARTWATCH_PHASE "3" CACHE STRING "Select firmware phase: 0=HC05, 1=OLED, 2=MPU6050, 3=Bluetooth")
set_property(CACHE SMARTWATCH_PHASE PROPERTY STRINGS 0 1 2 3)

set(FIRMWARE_TARGET SmartwatchFirmware)
set(SMARTWATCH_USE_WRAPPED_MAIN ON)
add_executable(${FIRMWARE_TARGET})

target_sources(${FIRMWARE_TARGET} PRIVATE
    src/main.c
)

target_compile_definitions(${FIRMWARE_TARGET} PRIVATE SMARTWATCH_PHASE=${SMARTWATCH_PHASE})

target_include_directories(${FIRMWARE_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(SMARTWATCH_PHASE STREQUAL "0")
    set(PHASE_DIR "Phase0_HC05Setting")
    target_sources(${FIRMWARE_TARGET} PRIVATE
        src/phase0_entry.c
        ${PHASE_DIR}/Core/Src/hc05_at.c
    )
elseif(SMARTWATCH_PHASE STREQUAL "1")
    set(PHASE_DIR "Phase1_OLED")
    target_sources(${FIRMWARE_TARGET} PRIVATE
        src/phase1_entry.c
        ${PHASE_DIR}/Core/Src/oled.c
        ${PHASE_DIR}/Core/Src/smartwatch_ui.c
    )
elseif(SMARTWATCH_PHASE STREQUAL "2")
    set(PHASE_DIR "Phase2_MPU6050")
    target_sources(${FIRMWARE_TARGET} PRIVATE
        src/phase2_entry.c
        ${PHASE_DIR}/Core/Src/oled.c
        ${PHASE_DIR}/Core/Src/smartwatch_ui.c
        ${PHASE_DIR}/Core/Src/mpu6050.c
    )
elseif(SMARTWATCH_PHASE STREQUAL "3")
    set(PHASE_DIR "Phase3_Bluetooth")
    target_sources(${FIRMWARE_TARGET} PRIVATE
        src/phase3_entry.c
        ${PHASE_DIR}/Core/Src/oled.c
        ${PHASE_DIR}/Core/Src/smartwatch_ui.c
        ${PHASE_DIR}/Core/Src/mpu6050.c
        ${PHASE_DIR}/Core/Src/bluetooth.c
    )
else()
    message(FATAL_ERROR "Unsupported SMARTWATCH_PHASE: ${SMARTWATCH_PHASE}")
endif()

add_subdirectory(${PHASE_DIR}/cmake/stm32cubemx)
target_link_libraries(${FIRMWARE_TARGET} PRIVATE stm32cubemx)

target_include_directories(${FIRMWARE_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/${PHASE_DIR}/Core/Inc
)

add_custom_command(TARGET ${FIRMWARE_TARGET} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${FIRMWARE_TARGET}> ${CMAKE_BINARY_DIR}/${FIRMWARE_TARGET}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${FIRMWARE_TARGET}> ${CMAKE_BINARY_DIR}/${FIRMWARE_TARGET}.hex
    COMMENT "Generating binary and hex images for ST-Link flashing"
)
