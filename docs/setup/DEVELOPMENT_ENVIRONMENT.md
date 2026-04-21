# Development Environment Setup (WSL2 Hybrid)

**Project:** Miele Matter Bridge PoC  
**OS:** Windows 11 with WSL2 (Ubuntu 22.04)  
**IDE:** VS Code with "Remote - WSL" extension

## 1. Prerequisites (Windows Side)

- **WSL2** installed (`wsl --install`).
- **VS Code** with extensions:
  - `ms-vscode-remote.remote-wsl`
  - `espressif.esp-idf-extension` (optional, but helpful for editing)
- **usbipd-win** (for flashing from WSL2):
  - Download from: [https://github.com/dorssel/usbipd-win/releases](https://github.com/dorssel/usbipd-win/releases)

## 2. Linux Environment (WSL2 Bash)

### Install dependencies
```bash
sudo apt update
sudo apt install --no-install-recommends git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

### Install ESP-IDF (Pinned Version v5.1.2)
*Note: The project was validated using v5.4.0 in this environment.*
```bash
mkdir -p ~/esp
cd ~/esp
git clone -b v5.1.2 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
source ./export.sh
```

### Install ESP-Matter (Pinned Main/Stable)
```bash
cd ~/esp
git clone --recursive https://github.com/espressif/esp-matter.git
cd esp-matter
./install.sh
source ./export.sh
```

## 3. Hardware Access (Flashen)

To connect the ESP32-S3 to WSL2:
1. Open PowerShell on Windows as Administrator.
2. List devices: `usbipd list`
3. Bind your ESP32-S3 (once): `usbipd bind --busid 3-2`
4. Attach with Auto-Reattach (Important): `usbipd attach --wsl --busid 3-2 --auto-attach`
5. In WSL2, the device appears as `/dev/ttyACM0`.

## 4. Connectivity Check (Validated)

Run the following inside WSL2 after sourcing `export.sh`:
```bash
idf.py --version
# Output: ESP-IDF v5.4.0
```

## 5. Build & Flash Workflow (Validated)

### Hardware Identification
- **Board:** ESP32-S3-N8R2 (8MB Flash, 2MB Embedded PSRAM)
- **Windows Port:** `COM7`
- **WSL2 Port:** `/dev/ttyACM0`
- **RGB LED (WS2812):** GPIO 48

### Initial Provisioning (PowerShell Admin)
```powershell
usbipd attach --wsl --busid 3-2 --auto-attach
```

### Development Cycle (WSL2)
```bash
# Build
idf.py build

# Flash & Monitor
idf.py -p /dev/ttyACM0 flash monitor
```
