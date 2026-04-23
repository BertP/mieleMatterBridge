#pragma once

#include "ssd1306.h"
#include <stdint.h>
#include <string.h>

/**
 * @brief Premium Icons for the Miele Matter Bridge (8x8)
 */
namespace ui {

static const uint8_t icon_wifi[] = { 0x00, 0x70, 0x88, 0x11, 0x22, 0x44, 0x00, 0x00 };
static const uint8_t icon_matter[] = { 0x3C, 0x42, 0x99, 0xBD, 0xBD, 0x99, 0x42, 0x3C };
static const uint8_t icon_cloud[] = { 0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x7E, 0x3C, 0x18 };

/**
 * @brief Dishwasher Icon (16x16)
 */
static const uint8_t icon_dishwasher[] = {
    0xFF, 0xFF, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0xBF, 0xFD, 0xA0, 0x05, 0xA0, 0x05,
    0xA0, 0x05, 0xA0, 0x05, 0xA0, 0x05, 0xA0, 0x05, 0xBF, 0xFD, 0x80, 0x01, 0x80, 0x01, 0xFF, 0xFF
};

/**
 * @brief Draws the Zen Pairing Screen (One-Shot)
 */
inline void draw_pairing_screen(ssd1306_t* dev, const char* version, const char* qr_payload) {
    ssd1306_clear(dev);
    
    // Header
    ssd1306_draw_string(dev, (128 - 12*8)/2, 0, "PAIRING MODE");
    
    // Footer
    char ver_str[12];
    snprintf(ver_str, sizeof(ver_str), "v%s", version);
    ssd1306_draw_string(dev, (128 - strlen(ver_str)*8)/2, 7, ver_str);

    // Note: The QR matrix rendering happens in main.cpp because it needs the qrcodegen lib,
    // but the frame is defined here.
}

/**
 * @brief Draws the Success Screen
 */
inline void draw_success_screen(ssd1306_t* dev, const uint8_t* checkmark_bitmap) {
    ssd1306_clear(dev);
    ssd1306_draw_string(dev, (128 - 7*8)/2, 1, "SUCCESS");
    
    // Draw 32x32 checkmark (4 pages high)
    ssd1306_draw_bitmap(dev, 48, 2, 32, 4, checkmark_bitmap);
    
    ssd1306_draw_string(dev, (128 - 12*8)/2, 6, "SYSTEM READY");
}

/**
 * @brief Draws the Transparent Bridge Dashboard
 */
inline void draw_dashboard(ssd1306_t* dev, const char* version, bool wifi, bool matter, bool cloud, const char* ip_addr) {
    ssd1306_clear(dev);
    
    // 1. Top Bar: Status Icons & Version
    if (wifi) ssd1306_draw_bitmap(dev, 2, 0, 8, 1, icon_wifi);
    if (matter) ssd1306_draw_bitmap(dev, 14, 0, 8, 1, icon_matter);
    if (cloud) ssd1306_draw_bitmap(dev, 26, 0, 8, 1, icon_cloud);
    
    char ver_str[12];
    snprintf(ver_str, sizeof(ver_str), "v2.3.0");
    ssd1306_draw_string(dev, 128 - (strlen(ver_str) * 8) - 2, 0, ver_str);
    
    // 2. Decorative Separator
    for (int x = 0; x < 128; x += 16) {
        ssd1306_draw_string(dev, x, 1, "."); 
    }

    // 3. Central Bridge Status (Massive Text)
    ssd1306_draw_string(dev, (128 - 13*8)/2, 3, "BRIDGE ONLINE");
    
    // 4. IP Info Area (Footer)
    if (ip_addr) {
        char ip_str[24];
        snprintf(ip_str, sizeof(ip_str), "IP: %s", ip_addr);
        ssd1306_draw_string(dev, (128 - strlen(ip_str)*8)/2, 6, ip_str);
    }
}

} // namespace ui
