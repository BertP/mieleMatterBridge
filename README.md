# 🚀 Miele Matter Bridge

Connecting Miele Cloud-based appliances to the Matter ecosystem via a dedicated ESP32-S3 bridge.

![License](https://img.shields.io/badge/License-MIT-green.svg)
![Version](https://img.shields.io/badge/Version-2.1.0-blue.svg)
![Target](https://img.shields.io/badge/Target-ESP32--S3-orange.svg)

## 📌 Project Overview
This project implements a bridge between the **Miele 3rd Party API** and the **Matter protocol**. It allows Miele appliances (currently focusing on Dishwashers) to be controlled and monitored via any Matter-compliant controller (Apple Home, Google Home, Home Assistant, etc.).

### Key Features
- **Matter Aggregator:** Exposes appliances as bridged endpoints.
- **Dynamic Commissioning:** On-screen QR-code generation for easy pairing.
- **Cloud-to-Local Sync:** Real-time synchronization of appliance states.
- **Diagnostic UI:** Integrated OLED display for status and pairing information.
- **Heartbeat Feedback:** Subtle RGB LED indicators for system health.

## 🛠 Hardware Specification
- **SoC:** ESP32-S3-N8R2 (8MB Flash, 2MB PSRAM)
- **Display:** SSD1306 OLED (I2C: SDA=8, SCL=9)
- **Status LED:** WS2812 RGB (GPIO 48)
- **Interface:** CH343 USB-to-Serial

## 🚀 Quick Start (Developer Setup)
This project uses a **hybrid WSL2/Windows** development environment.

### Prerequisites
1. **WSL2 (Ubuntu 22.04)** with ESP-IDF v5.4 and ESP-Matter SDK installed.
2. **usbipd-win** on Windows for serial passthrough.

### Build & Flash
```bash
# Attach the device (Windows)
usbipd attach --wsl --busid <BUSID> --auto-attach

# Build (WSL2)
export ESP_MATTER_PATH=/path/to/esp-matter
source $IDF_PATH/export.sh
source $ESP_MATTER_PATH/export.sh
idf.py build

# Flash & Monitor
idf.py -p /dev/ttyACM0 flash monitor
```

## 📂 Documentation
For detailed technical information, please refer to the `docs/` directory:
- [Mission Control (Ground Truth)](MISSION_CONTROL.md) - **Read this first!**
- [Handover Phase 1](docs/HANDOVER_PHASE_1.md)
- [Hardware Setup](docs/hardware/TARGET_HARDWARE.md)
- [Mapping Specification](docs/mapping/matter_miele_dishwasher_mapping.md)

## ⚖️ License
This project is licensed under the MIT License - see the LICENSE file for details.

---
*Created with ❤️ by the Miele Matter Bridge Team.*
