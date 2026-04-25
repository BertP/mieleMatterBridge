idf_component_register(SRCS "main.cpp" "miele_matter_mapping.cpp"
                    INCLUDE_DIRS "."
                    PRIV_INCLUDE_DIRS "." 
                                      "${ESP_MATTER_PATH}/examples/common/utils"
                                      "${ESP_MATTER_PATH}/components/esp_matter"
                                      "${ESP_MATTER_PATH}/components/esp_matter/data_model/generated/device_types/dish_washer_device"
                                      "${ESP_MATTER_PATH}/components/esp_matter/data_model/generated/device_types/aggregator_device"
                    PRIV_REQUIRES led_strip esp_matter chip esp_matter_bridge esp_matter_console miele_client esp_http_server mdns ssd1306_simple qrcode
                    EMBED_FILES www/index.html
                                www/assets/miele.svg
                                www/assets/matter.svg)

add_definitions(-DCHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID=0xFFF1)
add_definitions(-DCHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID=0x8001)
add_definitions(-DCHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME="Miele & Cie. KG")
add_definitions(-DCHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME="Miele Matter Bridge")

target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
