cmake_minimum_required(VERSION 3.5)

# 1. ESP-Matter Path & Device Configuration
if(NOT DEFINED ENV{ESP_MATTER_PATH})
    message(FATAL_ERROR "Please set ESP_MATTER_PATH to the path of esp-matter repo")
endif()

set(ESP_MATTER_PATH $ENV{ESP_MATTER_PATH})
set(MATTER_SDK_PATH ${ESP_MATTER_PATH}/connectedhomeip/connectedhomeip)

# Force ESP32-S3 DevKit configuration
set(ENV{ESP_MATTER_DEVICE_PATH} ${ESP_MATTER_PATH}/device_hal/device/esp32s3_devkit_c)

set(PROJECT_VER "2.3.0")
set(PROJECT_VER_NUMBER 230)
set(PROJECT_CODENAME "Transparent Zen")

# 2. Boilerplate Includes
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include(${ESP_MATTER_PATH}/examples/common/cmake_common/components_include.cmake)
include($ENV{ESP_MATTER_DEVICE_PATH}/esp_matter_device.cmake)

# 3. Component Directories
# We explicitly include the driver directories as components
set(EXTRA_COMPONENT_DIRS
    "${ESP_MATTER_PATH}/examples/common"
    "${MATTER_SDK_PATH}/config/esp32/components"
    "${ESP_MATTER_PATH}/components"
    "${ESP_MATTER_PATH}/device_hal/device"
    "${ESP_MATTER_PATH}/device_hal/button_driver/iot_button"
    "${ESP_MATTER_PATH}/device_hal/led_driver")

project(miele_matter_bridge)

# 4. Matter-specific Compiler Flags
idf_build_set_property(CXX_COMPILE_OPTIONS "-std=gnu++17;-Os;-DCHIP_HAVE_CONFIG_H;-Wno-overloaded-virtual;-Wno-error=unused-result" APPEND)
idf_build_set_property(C_COMPILE_OPTIONS "-Os;-Wno-error=unused-result" APPEND)
