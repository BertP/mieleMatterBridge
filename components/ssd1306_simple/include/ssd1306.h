#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1306_WIDTH  128
#define SSD1306_HEIGHT 64

typedef struct {
    i2c_port_t i2c_port;
    uint8_t i2c_addr;
} ssd1306_t;

/**
 * @brief Initialisiert das SSD1306 Display
 */
esp_err_t ssd1306_init(ssd1306_t *dev, i2c_port_t port, uint8_t addr);

/**
 * @brief Löscht das Display
 */
esp_err_t ssd1306_clear(ssd1306_t *dev);

/**
 * @brief Schreibt Text an eine Position (x, y)
 * y in Pages (0-7)
 */
esp_err_t ssd1306_draw_string(ssd1306_t *dev, uint8_t x, uint8_t y, const char *str);

#ifdef __cplusplus
}
#endif
