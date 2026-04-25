# 🔍 Analyse-Bericht: Google Home "TEST_VENDOR" Caching- & Identitäts-Konflikt

**Projekt:** Miele Matter Bridge (v2.3.9)  
**Datum:** 25. April 2026  
**Zielgruppe:** Externe Matter- & Smart-Home-Experten  

---

## 1. Problembeschreibung
Trotz expliziter Hard-Overrides der Identitäts-Strings im Quellcode (`Miele & Cie. KG` als Vendor, `Miele Matter Bridge` als Product) und dem erfolgreichen Deployment der Firmware-Version **v2.3.9** zeigt das Google Home Ökosystem (in den technischen Geräteinformationen) weiterhin folgende Werte an:
* **Hersteller:** `TEST_VENDOR`
* **Modell:** `TEST_PRODUCT`
* **Softwareversion:** `2.3.7` (obwohl das Gerät auf v2.3.9 läuft)

Dies führt dazu, dass das Branding der Bridge fehlschlägt und Endnutzer mit kryptischen Test-Bezeichnungen konfrontiert werden.

---

## 2. Technische Ursachen-Analyse

Es gibt zwei voneinander unabhängige, sich jedoch verstärkende Kernprobleme:

### A. Aggressives Cloud- & App-Caching von Google Home
Die Tatsache, dass Google Home für die Softwareversion immer noch `2.3.7` anzeigt, beweist zweifelsfrei, dass die App **überhaupt keine Live-Abfrage** der Cluster-Attribute des Geräts durchführt. 
* **Mechanismus:** Bei der allerersten Matter-Commissioning-Phase (Pairing) liest Google Home die Attribute des `Basic Information Clusters` (Endpoint 0) aus und speichert sie persistent in der Cloud ab, verknüpft mit der Node-ID / Fabric.
* **Verhalten:** Selbst bei einem vollständigen Re-Flash (`erase-flash`), einer Erhöhung der PID (`0x8000` -> `0x8001`) und Code-seitigen Overrides fragt Google Home die Daten nicht erneut vom Gerät ab, sondern greift auf die alte, gecachte Geräte-Historie zurück.

### B. Test-Zertifikate (DAC/PAI) vs. Production VID
Matter erzwingt eine kryptografische Absicherung der Geräte-Identität über eine Zertifikatskette:
* *Device Attestation Certificate (DAC)* -> *Product Attestation Intermediate (PAI)* -> *Product Attestation Authority (PAA)*.
* **Das Dilemma:** Da für die Bridge derzeit Test-Zertifikate verwendet werden, ist die Vendor-ID auf den offiziellen Test-Bereich festgelegt (`0xFFF1`). 
* Nutzt man die echte Miele-VID (`0x120E`) ohne passende Produktions-Zertifikate, bricht der Matter-Security-Handshake beim Pairing sofort ab.
* Nutzt man `0xFFF1`, mappt Google Home dieses Gerät in seiner internen Datenbank hart auf die Bezeichnung `TEST_VENDOR`.

---

## 3. Durchgeführte Lösungsversuche & Code-Maßnahmen

Folgende Strategien wurden im Code bereits implementiert ("Gerichtsfester Stand"):
1. **Zentrale Makro-Bereinigung (`CHIPProjectConfig.h`):** Saubere Definition von `CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME`, `PRODUCT_NAME`, etc.
2. **Laufzeit-Overrides via `DeviceInstanceInfoProvider`:** Implementierung einer Custom-Klasse in `main.cpp`, welche die Standard-Getter des Frameworks abfängt und die gewünschten Miele-Strings zurückgibt.
3. **Direktes Cluster-Update:** Manuelle Manipulation des Basic Information Clusters nach dem Start des Matter-Stacks via `esp_matter::attribute::update`.

---

## 4. Offene Fragestellungen für Experten

Um diesen Zustand zu durchbrechen, bitten wir um Einschätzungen zu folgenden Punkten:

1. **Cache-Busting:** Wie lässt sich der Google-Home-Gerätecache für Matter-Entwicklungsboards verlässlich und ohne Löschung des gesamten "Zuhauses" (Home) invalidieren? Gibt es bekannte Bluetooth/GATT-Cache-Probleme unter Android, die hier mitspielen?
2. **Branding trotz Test-VID (`0xFFF1`):** Erlaubt Google Home überhaupt ein Custom-Branding (Name/Hersteller) für Geräte, die sich über die Test-Vendor-ID ausweisen, oder ist `TEST_VENDOR` für diese VID im Google-Ökosystem fest verdrahtet?
3. **Entwickler-Zertifikate:** Gibt es für Prototyping-Phasen einen Weg, selbstsignierte Zertifikate für die VID `0x120E` in Google Home als "trusted" zu hinterlegen, um den echten Miele-Handshake zu testen?

---
*Erstellt durch die KI-Assistenz der Miele Matter Bridge Entwicklung.*
