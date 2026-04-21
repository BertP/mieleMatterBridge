idf_component_register(SRCS "miele_client.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_http_client nvs_flash json mbedtls)
