#include <cstdint>
#include <cstring>

static constexpr int DISP_W = 128;
static constexpr int DISP_H = 64;

static constexpr int QR_SIZE = 33;                 // inkl. Quiet Zone
static constexpr int QR_X0 = (DISP_W - QR_SIZE) / 2;  // 47
static constexpr int QR_Y0 = (DISP_H - QR_SIZE) / 2;  // 15

// 128x64 Monochrom-Framebuffer: 128 * 64 / 8 = 1024 Bytes
// Annahme: page-basiert, 8 vertikale Pixel pro Byte.
// Wenn deine SSD1306-Library anders packt, musst du nur fb_set_pixel anpassen.
static uint8_t g_fb[DISP_W * DISP_H / 8];

static constexpr uint64_t QR[QR_SIZE] = {
    0x000000000ULL,
    0x000000000ULL,
    0x000000000ULL,
    0x000000000ULL,
    0x01FC147F0ULL,
    0x010538410ULL,
    0x0174B25D0ULL,
    0x0174085D0ULL,
    0x0175695D0ULL,
    0x010486410ULL,
    0x01FD557F0ULL,
    0x000032000ULL,
    0x015411120ULL,
    0x002B7A050ULL,
    0x0056A55C0ULL,
    0x01DADEA50ULL,
    0x0056C59E0ULL,
    0x005126ED0ULL,
    0x015F097C0ULL,
    0x00E922110ULL,
    0x010C91F60ULL,
    0x0001F9150ULL,
    0x01FC37550ULL,
    0x01041D1D0ULL,
    0x017527F00ULL,
    0x017427B60ULL,
    0x017509D10ULL,
    0x0104639B0ULL,
    0x01FD11D50ULL,
    0x000000000ULL,
    0x000000000ULL,
    0x000000000ULL,
    0x000000000ULL
};

static inline void fb_set_pixel(uint8_t* fb, int x, int y, bool on)
{
    if (x < 0 || x >= DISP_W || y < 0 || y >= DISP_H) {
        return;
    }

    const int byte_index = x + (y / 8) * DISP_W;
    const uint8_t bit = 1u << (y & 7);

    if (on) {
        fb[byte_index] |= bit;
    } else {
        fb[byte_index] &= ~bit;
    }
}

static void fb_clear(uint8_t* fb)
{
    std::memset(fb, 0x00, DISP_W * DISP_H / 8);
}

static void draw_qr_into_fb(uint8_t* fb)
{
    fb_clear(fb);

    for (int row = 0; row < QR_SIZE; ++row) {
        const uint64_t bits = QR[row];
        for (int col = 0; col < QR_SIZE; ++col) {
            const bool black = (bits & (1ULL << (32 - col))) != 0;
            if (black) {
                fb_set_pixel(fb, QR_X0 + col, QR_Y0 + row, true);
            }
        }
    }
}