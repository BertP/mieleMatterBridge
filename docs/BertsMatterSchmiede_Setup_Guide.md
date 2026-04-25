# BertsMatterSchmiede — Matter Bridge Entwicklungs-Setup Guide

**Projekt:** Persönliche Matter Bridge Entwicklung  
**Entwickler:** Bert  
**Branding:** BertsMatterSchmiede  
**Stand:** April 2026  

---

## 1. Ausgangslage & Ziel

Dieses Dokument beschreibt, wie das Miele Matter Bridge Projekt vollständig auf persönliches Entwickler-Branding umgestellt wird — ohne CSA-Mitgliedschaft, ohne offizielle Zertifikate, rein auf Basis der öffentlich verfügbaren Matter Test-Infrastruktur.

**Ziel:** Ein funktionierendes Matter-Gerät, das sich in Google Home / Apple Home als `BertsMatterSchmiede` präsentiert, inklusive korrekter Softwareversion — statt `TEST_VENDOR`.

---

## 2. Alle Strings ersetzen — Komplette Übersicht

### 2.1 CHIPProjectConfig.h

```cpp
// ============================================================
// BertsMatterSchmiede — CHIPProjectConfig.h
// ============================================================

// Vendor
#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME        "BertsMatterSchmiede"
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID          0xFFF1   // Test-VID (kein CSA-Membership)
#endif

// Product
#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME       "Berts Matter Bridge"
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID         0x8001
#endif

// Software-Version
#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION   1
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING  "1.0.0-bert"
#endif

// Hardware-Version
#ifndef CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION
#define CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION   1
#endif

#ifndef CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION_STRING
#define CHIP_DEVICE_CONFIG_DEVICE_HARDWARE_VERSION_STRING  "ESP32-S3-DevKit"
#endif

// Serial Number (optional, aber hilfreich für Debugging)
#ifndef CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER
#define CHIP_DEVICE_CONFIG_TEST_SERIAL_NUMBER        "BERT-BRIDGE-001"
#endif

// Commissioning — Test-Setup-Codes (NICHT für Produktion)
#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE   20202021
#endif

#ifndef CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR  0xF00
#endif

// Failsafe
#ifndef CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC
#define CHIP_DEVICE_CONFIG_FAILSAFE_EXPIRY_LENGTH_SEC  60
#endif
```

> **Wichtig:** Alle Defines mit `#ifndef` Guard — damit Framework-Defaults nicht überschrieben werden, wenn das Symbol schon gesetzt ist, und keine Doppel-Defines entstehen.

---

### 2.2 sdkconfig.defaults

```ini
# BertsMatterSchmiede — sdkconfig.defaults
# Vendor & Product Strings
CONFIG_CHIP_DEVICE_VENDOR_NAME="BertsMatterSchmiede"
CONFIG_CHIP_DEVICE_PRODUCT_NAME="Berts Matter Bridge"

# IDs
CONFIG_CHIP_DEVICE_VENDOR_ID=0xFFF1
CONFIG_CHIP_DEVICE_PRODUCT_ID=0x8001

# Software-Version
CONFIG_CHIP_DEVICE_SOFTWARE_VERSION=1
CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING="1.0.0-bert"
```

---

### 2.3 main.cpp — Basic Information Cluster Update

Nach dem Start des Matter-Stacks die Attribute manuell setzen:

```cpp
// Nach esp_matter::start() aufrufen:
static void setBrandingAttributes() {
    // Endpoint 0 = Root Node / Basic Information Cluster (0x0028)
    esp_matter::attribute::update(0, 0x0028, 0x0001, // VendorName
        esp_matter::val_t { .val = { .s = "BertsMatterSchmiede" } });

    esp_matter::attribute::update(0, 0x0028, 0x0003, // ProductName
        esp_matter::val_t { .val = { .s = "Berts Matter Bridge" } });

    esp_matter::attribute::update(0, 0x0028, 0x000A, // SoftwareVersionString
        esp_matter::val_t { .val = { .s = "1.0.0-bert" } });
}
```

---

## 3. Google Home Developer Console — Konfiguration

### 3.1 Projekt anlegen

1. Öffne: https://console.home.google.com
2. Neues Projekt: **"BertsMatterSchmiede Dev"**
3. Matter Integration hinzufügen:
   - **Vendor ID:** `0xFFF1` (65521 dezimal)
   - **Product ID:** `0x8001` (32769 dezimal)
   - **Device Type:** Matter Bridge (`0x000E`)
   - **Brand Name:** `BertsMatterSchmiede`
   - **Product Name:** `Berts Matter Bridge`

### 3.2 Tester-Account freischalten

Unter **"Testing" → "Testers"** deine Google-Account-Email eintragen.  
Nur freigeschaltete Accounts sehen das Custom-Branding beim Onboarding.

---

## 4. Das TEST_VENDOR Problem — Warum bleibt es trotzdem?

### Was funktioniert mit VID 0xFFF1 + Developer Console:
- ✅ **Onboarding-Screen** zeigt "BertsMatterSchmiede" beim Pairing
- ✅ **Geräte-Name** in der Hauptansicht ist konfigurierbar
- ✅ **Matter-Commissioning** funktioniert vollständig

### Was NICHT funktioniert mit VID 0xFFF1:
- ❌ **Technische Informationen → Hersteller** bleibt `TEST_VENDOR`
- ❌ **Technische Informationen → Modell** bleibt `TEST_PRODUCT`
- ❌ **Softwareversion** zeigt den gecachten Wert, nicht den aktuellen

**Ursache:** Google Home hat für die Test-VID-Range `0xFFF1–0xFFF4` eine harte Hardcodierung für die Detail-Ansicht. Das ist by Design — kein Bug, keine Fehlkonfiguration.

---

## 5. Lösung: Eigene VID mit selbstsignierten Zertifikaten

### 5.1 Freie Test-VID wählen

Für persönliche Entwicklung ohne CSA-Mitgliedschaft kann `0xFFF1` genutzt werden — das reicht für alle funktionalen Tests. Für echtes Branding auch in den technischen Details gibt es zwei Wege:

**Option A — Bleibt bei 0xFFF1 (einfachste Lösung für Entwicklung):**  
Akzeptiere `TEST_VENDOR` in den Technischen Informationen. Das Gerät funktioniert korrekt — es ist nur Kosmetik.

**Option B — Eigene VID mit chip-cert (für echtes Branding):**

### 5.2 chip-cert Zertifikate generieren

```bash
# 1. CHIP SDK klonen (falls noch nicht vorhanden)
git clone --recurse-submodules https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

# 2. Umgebung aktivieren
source scripts/activate.sh

# 3. chip-cert bauen
cd src/credentials
gn gen out
ninja -C out chip-cert

# 4. Zertifikate generieren für Bert
export BERT_VID=0xFFF1    # oder eine eigene freie VID z.B. 0xFFF1
export BERT_PID=0x8001

# PAI generieren
./out/chip-cert gen-att-cert --type i \
  --subject-cn "BertsMatterSchmiede PAI" \
  --subject-vid "${BERT_VID}" \
  --valid-from "2024-01-01 00:00:00" \
  --lifetime 3650 \
  --ca-key test/attestation/Chip-Test-PAA-NoVID-Key.pem \
  --ca-cert test/attestation/Chip-Test-PAA-NoVID-Cert.pem \
  --out-key test/attestation/Bert-PAI-Key.pem \
  --out test/attestation/Bert-PAI-Cert.pem

# DAC generieren
./out/chip-cert gen-att-cert --type d \
  --subject-cn "BertsMatterSchmiede DAC" \
  --subject-vid "${BERT_VID}" \
  --subject-pid "${BERT_PID}" \
  --valid-from "2024-01-01 00:00:00" \
  --lifetime 3650 \
  --ca-key test/attestation/Bert-PAI-Key.pem \
  --ca-cert test/attestation/Bert-PAI-Cert.pem \
  --out-key test/attestation/Bert-DAC-Key.pem \
  --out test/attestation/Bert-DAC-Cert.pem
```

---

## 6. Cache-Busting — So erzwingst du frische Daten in Google Home

Das ist der wichtigste Schritt, damit Änderungen auch wirklich sichtbar werden:

```
Schritt 1: Google Home App → Gerät antippen → ⚙️ Einstellungen → "Gerät entfernen"
           (NICHT nur "Gerät zurücksetzen" — komplett entfernen!)

Schritt 2: ESP32-S3 vollständig flashen:
           idf.py erase-flash
           idf.py flash

Schritt 3: Android Bluetooth-Cache leeren:
           Einstellungen → Apps → alle Apps anzeigen → Bluetooth → Speicher → Cache leeren

Schritt 4: Gerät neu koppeln (Pairing von Null)
           → Google Home liest beim ersten Commissioning alle Attribute frisch ein
```

**Warum ist Schritt 1 kritisch?**  
`erase-flash` allein löscht NUR den ESP32-Flash. Google Home behält den Cloud-Cache der alten Fabric-ID. Nur ein vollständiges Entfernen aus der App löscht den Server-seitigen Cache.

---

## 7. Schnell-Referenz: Welche Strings wo

| String | Datei | Key/Makro |
|--------|-------|-----------|
| Vendor Name (Code) | `CHIPProjectConfig.h` | `CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME` |
| Product Name (Code) | `CHIPProjectConfig.h` | `CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME` |
| Vendor Name (Build) | `sdkconfig.defaults` | `CONFIG_CHIP_DEVICE_VENDOR_NAME` |
| Product Name (Build) | `sdkconfig.defaults` | `CONFIG_CHIP_DEVICE_PRODUCT_NAME` |
| Vendor Name (Runtime) | `main.cpp` | `esp_matter::attribute::update(0, 0x0028, 0x0001, ...)` |
| Product Name (Runtime) | `main.cpp` | `esp_matter::attribute::update(0, 0x0028, 0x0003, ...)` |
| SW-Version String | `main.cpp` | `esp_matter::attribute::update(0, 0x0028, 0x000A, ...)` |
| Google-Branding | Developer Console | Brand Name + Product Name |

---

## 8. Erwartetes Verhalten nach korrektem Setup

| Wo | Was du siehst | Warum |
|----|--------------|-------|
| Onboarding-Screen | "BertsMatterSchmiede" | Developer Console Branding |
| Gerätename (Hauptansicht) | "Berts Matter Bridge" | Konfigurierbar |
| Technische Info → Hersteller | `TEST_VENDOR` | VID 0xFFF1 Hardcodierung by Google |
| Technische Info → Modell | `TEST_PRODUCT` | Gleiche Ursache |
| Firmware-Log (Serial) | Korrekte Strings | Cluster-Attribute gesetzt |
| chip-tool Readout | Korrekte Strings | Matter-Protokoll korrekt |

---

## 9. Nächste Schritte (Roadmap)

1. **Jetzt:** `CHIPProjectConfig.h` und `sdkconfig.defaults` mit Bert-Strings anpassen
2. **Dann:** Gerät komplett aus Google Home entfernen + neu flashen + neu pairen
3. **Optional:** Google Home Developer Console Projekt mit `0xFFF1` / `0x8001` anlegen
4. **Wenn echtes Branding gewünscht:** CSA Basic Membership (~$3.500/Jahr) für eigene VID

---

*Erstellt für Bert — persönliches Matter-Entwicklungsprojekt, April 2026*
