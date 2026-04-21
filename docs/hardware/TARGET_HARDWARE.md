# Target Hardware Specification: ESP32-S3-N8R2

**Project:** Miele Matter Bridge PoC  
**Hardware Status:** ESP32-S3-DevKit-like Board
**Revision:** 1.1 (OLED added 2026-04-21)

## 1. Core Module Details

- **SoC:** ESP32-S3 (Dual-Core, up to 240 MHz)
- **Flash:** 8 MB (Quad SPI)
- **PSRAM:** 2 MB (Embedded Quad SPI)
- **Antenna:** PCB Antenna

## 2. Resource Budgeting

| Resource | Allocated for PoC | Purpose |
|---|---|---|
| **Flash Partition (APP)** | ~3 MB | Main Firmware & Matter Stack |
| **NVS Partition** | 1 MB | Settings, Tokens, Matter Fabric |
| **Heap (Internal)** | ~256 KB | Fast OS Operations |
| **Heap (PSRAM)** | ~1.5 MB | TLS Buffers, JSON Parsing, Matter Table |

## 3. Peripheral Mapping (GPIOs) - Validated

| Component | GPIO | Mode | Description |
|---|---|---|---|
| **RGB LED (WS2812)** | 48 | Output | Status Indication (Onboard) |
| **OLED SDA** | 8 | I/O | I2C Data (SSD1306) |
| **OLED SCL** | 9 | Output | I2C Clock (SSD1306) |
| **Action Button** | 0 | Input | Boot / Factory Reset (Onboard) |
| **USB D- (Serial)** | 19 | I/O | Built-in USB |
| **USB D+ (Serial)** | 20 | I/O | Built-in USB |
| **UART TX** | 43 | Output | Debug Logging |
| **UART RX** | 44 | Input | Debug / CLI |

## 4. Power & Connectivity

- **Power Supply:** 5V via USB-C (left port for Serial/JTAG)
- **Matter Transport:** Wi-Fi 2.4 GHz
- **Commissioning:** BLE (Bluetooth Low Energy 5.0)

## 5. Known Hardware Limitations / Constraints

- [x] USB-Serial enumeration: Works via `usbipd` (BUSID 3-2 validated).
- [x] RGB LED requires `led_strip` component (verified at GPIO 48).
- [x] OLED Display requires I2C (Bus 0, Pins 8/9).
- [ ] Pins that are strapped and should be avoided?

---
*Status: Toolchain and basic I/O validated. Architecture expanded for Headless Status Display.*
