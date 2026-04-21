# Target Hardware Specification: ESP32-S3-N8R2

**Project:** Miele Matter Bridge PoC  
**Hardware Status:** [z.B. DevKit / Custom Board]  
**Revision:** 1.0

## 1. Core Module Details

- **SoC:** ESP32-S3 (Dual-Core, up to 240 MHz)
- **Flash:** 8 MB (Quad SPI)
- **PSRAM:** 2 MB (Quad SPI)
- **Antenna:** PCB / External [Bitte spezifizieren]

## 2. Resource Budgeting

| Resource | Allocated for PoC | Purpose |
|---|---|---|
| **Flash Partition (APP)** | ~3 MB | Main Firmware & Matter Stack |
| **NVS Partition** | 1 MB | Settings, Tokens, Matter Fabric |
| **Heap (Internal)** | ~256 KB | Fast OS Operations |
| **Heap (PSRAM)** | ~1.5 MB | TLS Buffers, JSON Parsing, Matter Table |

## 3. Peripheral Mapping (GPIOs)

| Component | GPIO | Mode | Description |
|---|---|---|---|
| **Status LED (Blue)** | [Pin] | Output | Matter Connection State |
| **Action Button** | [Pin] | Input | Factory Reset / Manual Sync |
| **UART TX** | 43 | Output | Debug Logging |
| **UART RX** | 44 | Input | Debug / CLI |

## 4. Power & Connectivity

- **Power Supply:** 5V via USB / External
- **Matter Transport:** Wi-Fi 2.4 GHz
- **Commissioning:** BLE (Bluetooth Low Energy 5.0)

## 5. Known Hardware Limitations / Constraints

- [ ] USB-Serial enumeration issues?
- [ ] Known power-hungry components?
- [ ] Pins that are strapped and should be avoided?

---
*Bitte ergänze die mit [ ] markierten Stellen entsprechend deiner Hardware-Dokumentation.*
