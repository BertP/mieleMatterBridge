# Matter QR-Code → 128×64 SSD1306 (ESP-IDF, C++)

## 📦 Voraussetzung

Du brauchst die QR-Code Library von Nayuki:  
https://github.com/nayuki/QR-Code-generator (C++ Version)

Kopiere diese Dateien in dein Projekt:
- qrcodegen.hpp
- qrcodegen.cpp

---

## 🧠 Funktion: Matter Payload → Framebuffer

```cpp
#include <array>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include "qrcodegen.hpp"

using qrcodegen::QrCode;

static constexpr int DISP_W = 128;
static constexpr int DISP_H = 64;
static constexpr int FB_SIZE = DISP_W * DISP_H / 8;

// SSD1306: 8 vertikale Pixel pro Byte (Page Layout)
static inline void fbSetPixel(std::array<uint8_t, FB_SIZE>& fb, int x, int y, bool on)
{
    if (x < 0 || x >= DISP_W || y < 0 || y >= DISP_H) return;

    const int index = x + (y / 8) * DISP_W;
    const uint8_t bit = 1u << (y & 7);

    if (on) fb[index] |= bit;
    else    fb[index] &= ~bit;
}

static inline void fbClear(std::array<uint8_t, FB_SIZE>& fb)
{
    fb.fill(0x00);
}

// 👉 Hauptfunktion
std::array<uint8_t, FB_SIZE> makeMatterQrFramebuffer(const char* payload)
{
    std::array<uint8_t, FB_SIZE> fb{};
    fbClear(fb);

    // QR-Code erzeugen (LOW = maximal groß, MEDIUM = robuster)
    const QrCode qr = QrCode::encodeText(payload, QrCode::Ecc::LOW);

    const int qrSize = qr.getSize();   // ohne Quiet Zone
    const int quiet = 4;               // Standard Quiet Zone
    const int total = qrSize + quiet * 2;

    // Integer-Skalierung (wichtig: KEIN Stretching!)
    const int scaleX = DISP_W / total;
    const int scaleY = DISP_H / total;
    const int scale = std::max(1, std::min(scaleX, scaleY));

    // Zentrieren
    const int drawW = total * scale;
    const int drawH = total * scale;
    const int x0 = (DISP_W - drawW) / 2;
    const int y0 = (DISP_H - drawH) / 2;

    // Rendern
    for (int y = 0; y < qrSize; y++) {
        for (int x = 0; x < qrSize; x++) {

            if (!qr.getModule(x, y)) continue;

            const int px = x0 + (x + quiet) * scale;
            const int py = y0 + (y + quiet) * scale;

            for (int dy = 0; dy < scale; dy++) {
                for (int dx = 0; dx < scale; dx++) {
                    fbSetPixel(fb, px + dx, py + dy, true);
                }
            }
        }
    }

    return fb;
}