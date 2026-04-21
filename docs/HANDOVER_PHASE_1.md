# Übergabedokumentation: Phase 1 (Validierung & Smoke Test)

Dieses Dokument dient der Dokumentation des Projektstatus nach Abschluss der initialen Toolchain-Validierung für den **Miele Matter Bridge PoC**.

## 1. Status Quo (Stand 2026-04-21)

Die technische Basis für die Entwicklung ist erfolgreich etabliert. Ein funktionsfähiger "Smoke Test" (Blinky & RGB LED) wurde auf die Hardware geflasht.

### Kern-Erkenntnisse
- **Hardware:** Das Board ist ein **ESP32-S3-N8R2**. Die 8MB Flash und 2MB PSRAM wurden verifiziert.
- **RGB LED:** Die Onboard-RGB-LED befindet sich auf **GPIO 48** (WS2812).
- **USB-Interface:** Das Board nutzt einen CH343 USB-Serial Chip. Auf dem Host-System (Windows) wird es als **COM7** erkannt.

## 2. Entwicklungsumgebung (WSL2 Hybrid)

Das Projekt nutzt einen hybriden Workflow zwischen Windows (Host) und WSL2 (Build/Flash).

- **Build-Umgebung:** WSL2 (Ubuntu 22.04) mit **ESP-IDF v5.4.0**.
- **USB-Brücke:** `usbipd-win` wird verwendet, um das USB-Gerät von Windows an WSL2 durchzureichen.
- **Stabilität:** Der Flash-Vorgang wurde durch den `--auto-attach` Modus stabilisiert. Damit bleibt das Gerät auch nach einem Soft-Reset mit Linux verbunden.

### Wichtige Befehle für die Übergabe
| Aktion | Umgebung | Befehl |
|---|---|---|
| USB-Gerät binden | Windows (Admin) | `usbipd attach --wsl --busid 3-2 --auto-attach` |
| Projekt bauen | WSL2 | `idf.py build` |
| Flashen & Monitor | WSL2 | `idf.py -p /dev/ttyACM0 flash monitor` |

## 3. Quellcode-Struktur (Aktuell)

- `main/main.c`: Enthält den RGB-LED Smoke Test.
- `main/idf_component.yml`: Definiert die Abhängigkeit zum `led_strip` Treiber.
- `docs/`: Vollständige Projektdokumentation inklusive Hardware-Mapping und Setup-Guide.

## 4. Offene Punkte / Risiken für Phase 2 (Matter)

1. **Ressourcen:** Der Matter-Stack ist deutlich speicherhungriger als der Smoke Test. Das Partitionslayout wurde noch nicht finalisiert.
2. **Provisionierung:** Die Matter-Provisionierung (QR-Code) muss in die CI/CD oder den Monitor-Workflow integriert werden.
3. **Miele API:** Die Anbindung an die Miele API erfolgt in Phase 2 zunächst über einen Mock-Layer.

---
**Status: READY FOR MATTER INTEGRATION**
