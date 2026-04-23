# Developer Setup Guide: Miele Matter Bridge

This document outlines the local environment setup and tools required for development, debugging, and forensics.

## 1. Core Toolchain
- **ESP-IDF:** v5.1.2
- **ESP-Matter SDK:** Latest stable (aligned with IDF v5.1)
- **Target Hardware:** ESP32-S3 (8MB Flash, 2MB PSRAM)

## 2. API Credentials (Miele 3rd Party API)
Credentials are now managed via the ESP-IDF Kconfig system.
- **Location:** `main/Kconfig.projbuild`
- **Configuration:** Run `idf.py menuconfig` -> `Miele Bridge Configuration`
- **Local Overwrite:** You can set these directly in your local `sdkconfig` (git-ignored):
  ```ini
  CONFIG_MIELE_CLIENT_ID="your_client_id"
  CONFIG_MIELE_CLIENT_SECRET="your_client_secret"
  ```

## 3. Forensics & Debugging Tools
For analyzing commissioning issues (e.g., Fail-safe timer expired), logs from both the device and the commissioner (mobile app) are required.

### Android Debug Bridge (ADB)
Required for capturing logs from the Google Home App or Android system during pairing.
- **Known Good Path (Windows):** `C:\Users\bertp\Tools\scrcpy-win64-v3.3.4\adb.exe`
- **Usage:**
  ```powershell
  # Example: Capture Matter-related logs from Android
  & "C:\Users\bertp\Tools\scrcpy-win64-v3.3.4\adb.exe" logcat | Select-String "CHIP"
  ```

### Serial Monitor
- **Command:** `idf.py monitor`
- **Tip:** Use `WIFI_PS_NONE` in code (already implemented) to prevent BLE/Wi-Fi contention during pairing.

## 4. Matter Identity (Development)
The bridge uses dynamic QR code generation, but for manual pairing, the following defaults are used (fetched from stack):
- **Passcode:** `12345678` (fetched dynamically)
- **Discriminator:** `1234` (fetched dynamically)

## 5. Web Onboarding
The bridge hosts a local webserver for OAuth pairing.
- **Hostname:** `http://miele-bridge.local`
- **Fallback:** Check serial logs for the IP address.
