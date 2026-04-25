# 🚚 WSL Migration Guide: Miele Matter Bridge

This document describes the process of moving the project from the Windows host filesystem to the native WSL2 filesystem for improved performance and toolchain stability.

## 1. Rationale
- **Build Speed:** Up to 5x faster compilation on native `ext4`.
- **Git Stability:** No more filesystem translation overhead for git operations.
- **Toolchain Compatibility:** The ESP-IDF and Matter SDK are designed for native Linux environments.

## 2. Migration Steps

### Step A: Create Target Directory
Run this in PowerShell to ensure the base folder exists in WSL:
```powershell
wsl mkdir -p ~/projects/mieleMatterBridge
```

### Step B: Sync Files (Safe Transfer)
Run this command in PowerShell to sync the source code while excluding temporary build artifacts:
```powershell
wsl rsync -av --progress --exclude='build/' --exclude='managed_components/' --exclude='sdkconfig.old' --exclude='.gemini/' /mnt/c/Users/bertp/work/Antigravity/mieleMatterBridge/ ~/projects/mieleMatterBridge/
```

### Step C: Verify Transfer
Check the content in WSL:
```powershell
wsl ls -la ~/projects/mieleMatterBridge
```

### Step D: Open in VS Code (Remote WSL)
1. Open VS Code.
2. Press `Ctrl+Shift+P` -> **WSL: Connect to WSL**.
3. Open Folder: `/home/bert/projects/mieleMatterBridge`.

## 3. Post-Migration Checklist

### 1. Re-initialize Build System
Since the absolute paths in `sdkconfig` and CMake caches will change, a clean build is required:
```bash
cd ~/projects/mieleMatterBridge
export ESP_MATTER_PATH=/home/bert/esp/esp-matter
source /home/bert/esp/esp-idf/export.sh
source /home/bert/esp/esp-matter/export.sh

# Complete cleanup
idf.py fullclean
idf.py build
```

### 2. Update MISSION_CONTROL.md
Update the "System Environment" section in the new location to reflect the native WSL paths.

---
*Created: 2024-04-24*
