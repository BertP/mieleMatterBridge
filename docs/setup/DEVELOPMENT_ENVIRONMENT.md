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
3. Bind your ESP32-S3 (once): `usbipd bind --busid <BUSID>`
4. Attach to WSL2: `usbipd attach --wsl --busid <BUSID>`
5. In WSL2, the device should now appear as `/dev/ttyACM0` or `/dev/ttyUSB0`.

## 4. Connectivity Check (Smoke Test)

Run the following inside WSL2 after sourcing `export.sh`:
```bash
idf.py --version
# Should output: ESP-IDF v5.1.2
```

## 5. Build Smoke Test

To verify Matter build capability:
```bash
cd ~/esp/esp-matter/examples/light
idf.py set-target esp32s3
idf.py build
```
This build consumes significant resources and tests the entire toolchain (GN/Ninja/Project CHIP).
