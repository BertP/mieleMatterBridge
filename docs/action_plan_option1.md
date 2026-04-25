# 📋 Handlungsplan: Umsetzung Option 1 (Developer Console)

**Projekt:** Miele Matter Bridge  
**Status:** CODE STOPP aktiv 🛑  

Dieser Plan beschreibt die notwendigen Schritte, um das Google Home Caching-Problem mittels der **Google Home Developer Console** zu umgehen und ein korrektes Miele-Branding für Testgeräte zu erzielen.

---

## Phase 1: Cloud-Setup (Aktion: User)

Die folgenden Schritte müssen in der Google Home Infrastruktur durchgeführt werden:

1. **Developer Console aufrufen:**
   * Navigiere zu [console.home.google.com](https://console.home.google.com).

2. **Matter Integration erstellen:**
   * Erstelle ein neues Projekt bzw. eine neue Matter-Integration.
   * **Vendor ID (VID):** `0xFFF1` (Fest vorgegebene Test-VID).
   * **Product ID (PID):** `0x8001` (Frei wählbar, bitte für Phase 2 notieren).

3. **Branding & Sichtbarkeit:**
   * Hinterlege den Anzeigenamen (z.B. *Miele Matter Bridge*).
   * Trage die Google-E-Mail-Adressen aller beteiligten Entwickler/Tester in die **Tester-Liste** ein. *Ohne diesen Eintrag ignoriert Google Home das Branding!*

---

## Phase 2: Cache-Busting (Aktion: User)

Um sicherzustellen, dass Google Home die neuen Metadaten einliest:

1. **Gerät entfernen:**
   * Lösche die Bridge vollständig aus der Google Home App.
2. **Bluetooth-Cache leeren (Android):**
   * Einstellungen -> Apps -> Bluetooth -> Speicher -> Cache leeren.

---

## Phase 3: Firmware-Rückbau & Anpassung (Aktion: Antigravity)
*Hinweis: Wird erst nach Aufhebung des Code-Stopps ausgeführt.*

1. **Zertifikats-Bypass entfernen:**
   * Löschen von `main/matter_identity_fix.cpp`.
   * Entfernen der Dummy-Referenzen aus `main/main.cpp`.
2. **ID-Konfiguration:**
   * Setzen der VID `0xFFF1` und der gewählten PID in der `CHIPProjectConfig.h` sowie `sdkconfig.defaults`.

---

## Phase 4: Deployment

1. Vollständiger Flash-Wipe: `idf.py erase-flash`.
2. Sauberer Build und Flash mit Live-Logging (`logs/esp32_live.log`).
