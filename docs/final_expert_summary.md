# 📊 Abschluss-Zusammenfassung für die externe Expertise: Der Build-System-Bug

**Datum:** 25.04.2026  
**Fokus:** Warum Google Home trotz PID-Änderung die alten Daten gezogen hat.  

---

## 1. Die Beobachtung
Nachdem wir die PID im Code (`CHIPProjectConfig.h`, `sdkconfig.defaults` und `main.cpp`) auf `0xBE27` (48679) geändert und den Flash gewiped haben, zeigte Google Home beim erneuten Pairing *exakt* dasselbe Fehlerbild:
* Hersteller: `TEST_VENDOR`
* Softwareversion: `2.3.7`
* Gerätename: `TEST_PRODUCT`

---

## 2. Die technische Ursachenforschung (Der Kconfig-Quirk)
Ein genauer Blick in die Boot-Logs des ESP32 offenbart die Wurzel des Übels:

```text
// Log-Zeile 145 (Cluster Override greift)
esp_matter_attribute: Endpoint 0x0000's Attribute 0x00000004 is 48679 (0xBE27)

// Log-Zeile 140 (BLE Advertising!)
chip[DIS]: Advertise commission parameter vendorID=65521 productID=32769 (0x8001)
```

### Was ist passiert?
1. Wir haben die neue PID `0xBE27` in den Quellcodedateien und in der Template-Datei `sdkconfig.defaults` hinterlegt.
2. **Das Problem:** Das ESP-IDF Build-System aktualisiert die finale Arbeitsdatei `sdkconfig` **nicht** automatisch, wenn nur das Default-Template geändert wird. 
3. Die tatsächliche Kconfig-Variable `CONFIG_DEVICE_PRODUCT_ID` blieb in der `sdkconfig` auf `0x8001` eingefroren.
4. Da das Bluetooth-Advertising (BLE/mDNS) extrem früh aus Kconfig-Parametern konstruiert wird, hat der ESP32 weiterhin mit der alten PID `0x8001` gefunkt.

### Konsequenz
Google Home hat über Bluetooth die alte PID `0x8001` empfangen, sofort die alte Cloud-Cache-Schublade geöffnet und unserem Schwenk keine Chance gegeben.

---

## 3. Lösungsweg für den nächsten Durchlauf
Um die Konfiguration wirklich zu erzwingen, muss die Toolchain wie folgt aufgerufen werden:
```bash
idf.py reconfigure # Zwingt ESP-IDF, das Template neu einzulesen
idf.py erase-flash
idf.py build flash
```
Damit wird der Cache-Bruch auf `0xBE27` endlich wirksam.
