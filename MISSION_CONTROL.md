# 🚀 MISSION CONTROL: Technical Ground Truth

**THIS DOCUMENT IS THE PRIMARY CONTEXT FOR ALL AGENTS. READ THIS FIRST AT EVERY SESSION START.**

This file defines the immutable technical constraints, toolchains, and verified workflows for the Miele Matter Bridge project.

## 1. System Environment (Hybrid Setup)
- **Host OS:** Windows 11
- **Build/Flash OS:** WSL2 (Ubuntu 22.04)
- **Shell:** PowerShell (Host) / Bash (WSL2)
- **ESP-IDF Version:** v5.4.0 (Path: `/home/bert/esp/esp-idf`)

## 2. Hardware Specification
- **SoC:** ESP32-S3-N8R2 (8MB Flash, 2MB PSRAM)
- **USB Interface:** CH343 (recognized as **COM7** on Windows)
- **Onboard RGB LED:** GPIO 48 (WS2812)
- **OLED Display:** SSD1306 (I2C: SDA=8, SCL=9)

## 3. Verified Workflows (DO NOT DEVIATE)

### A. USB Passthrough (Windows Admin required)
Before flashing or monitoring in WSL2, the device must be attached:
```powershell
usbipd attach --wsl --busid 3-2 --auto-attach
```

### B. Build Firmware
Always run within the WSL2 context with BOTH IDF and Matter environments sourced:
```bash
export ESP_MATTER_PATH=/home/bert/esp/esp-matter && source /home/bert/esp/esp-idf/export.sh && source /home/bert/esp/esp-matter/export.sh && idf.py build
```

### C. Flash & Monitor
The device appears as `/dev/ttyACM0` in WSL2:
```bash
export ESP_MATTER_PATH=/home/bert/esp/esp-matter && source /home/bert/esp/esp-idf/export.sh && source /home/bert/esp/esp-matter/export.sh && idf.py -p /dev/ttyACM0 flash monitor
```

## 4. Human-AI Collaboration Rules (The "Bert-Protocol")
- **Active Thinking:** Do not just execute. Think about the implications. If a task seems risky, inconsistent, or if there's a better way, **speak up immediately.**
- **Proactive Feedback:**
### 1. Matter Connectivity (The Foundation)
- [x] **Commissioning:** COMPLETED 🟢 (Successfully paired with Google Home after NVS erase and full sync verified).
- [x] **Stability:** High. No more bootloops.
- [ ] **Discovery:** mDNS and BLE advertising are working correctly.

### 2. Miele API Integration
- [x] **Authentication:** OAuth Flow working.
- [x] **Synchronization:** "SYNC DONE" achieved.
- [ ] **Persistence:** Verify if Token survives a hard reboot (NVS storage check).
- [ ] **Appliance Mapping:** Map all dishwasher states (Running, Finished, Salt/RinseAid).

### 3. UI/UX & Aesthetics (Current Focus: "Zen Vision")
- [x] **OLED Redesign:** Transition to premium icon-based dashboard (v2.2.0).
- [ ] **Status Icons:** Visual indicators for Wi-Fi, Cloud, and Matter implemented.
- [ ] **Zen Mode:** Auto-dimming/switching off logic for OLED.
- [ ] **Version Display:** Versioning consistent across all interfaces.
If a requirement is missing or a technical debt is spotted, point it out.
- **Context Preservation:** Never ignore established documentation (HANDOVER, DAILY_LOG). If in doubt, ask for clarification instead of guessing.
- **No Passivity:** You are a partner in this project, not just a tool. "Mund aufmachen" (speak up) is a requirement.
- **Team-Responsibility:** Antigravity (AI) ist für das Auslösen der Build- und Flash-Vorgänge über das Terminal verantwortlich. Der Lead-Developer (Bert) stellt die physische Bereitschaft der Hardware sicher.

## 5. Key Configurations
- **Aktueller Status:** [DEPLOYING] 🚀 (v2.2.0 "Zen Vision")
- **Bootloop Teil 1 (LoadProhibited):** BEHOBEN. Ursache war der Zugriff auf Attribute vor `esp_matter::start()`.
- **Bootloop Teil 2 (Abort in QR-Code):** BEHOBEN (Fix im Build). Ursache war der Zugriff auf Matter-Provider vor Stack-Start.
- **mDNS (3000008):** Nächstes Ziel nach stabilem Boot.
- **Deployment:** v2.2.0 "Zen Vision" wird gerade geflasht.
- **Miele API Credentials:** Managed via Kconfig (stored in local `sdkconfig`).
- **WLAN Stabilität:** `esp_wifi_set_ps(WIFI_PS_NONE)` is mandatory for Matter commissioning.
- **Partitions:** Standard Matter partitions (8MB flash layout).

## 6. Verified Milestones
- [x] **2026-04-21:** Smoke Test successful (Blinky on GPIO 48).
- [x] **2026-04-22:** Phase 2.1 - **v2.1.0 "Dynamic Heartbeat"** (QR, Kconfig, LED Heartbeat).
- [x] **2026-04-23:** Phase 2.2 - **v2.2.0 "Zen Vision"** (Premium OLED Dashboard, Status Icons).

## 7. Versioning Policy
- **Semantic Versioning (SemVer):** MAJOR.MINOR.PATCH.
- **Phase 2 (Matter):** Version 2.x.x.
- **Codenames:** Every significant build receives an "aka" name (e.g., "The Golden Build", "Dynamic Heartbeat").

## 8. Autonomous Actions (The "Go-Ahead")
- **AI Assistance:** Der AI-Agent führt Build-, Flash- und Monitor-Befehle autonom aus, sobald ein Entwicklungsschritt abgeschlossen ist, und analysiert die Ergebnisse in Echtzeit.
- **Pre-Flash Check:** Always verify the serial port (`/dev/ttyACM0`) in WSL before flashing.


---
*Last Updated: 2026-04-22*
