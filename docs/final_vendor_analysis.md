# 📊 Technischer Abschlussbericht: Die "BertsMatterSchmiede" Cache-Falle

**Datum:** 25.04.2026  
**Fokus:** Warum Google Home trotz Firmware-Änderung alte Daten anzeigt  

---

## 1. Der Ist-Zustand (App vs. Realität)
Trotz vollständigem Schwenk auf die Firmware-Version `1.0.0-bert` und das Branding `BertsMatterSchmiede` zeigt Google Home weiterhin:
* Hersteller: `TEST_VENDOR`
* Modell: `TEST_PRODUCT`
* Softwareversion: `2.3.7`
* Gerätename: `TEST_PRODUCT (2)`

---

## 2. Was hätte sich ändern sollen? (Laut Setup-Guide)
Ziel der Umstellung war es zu prüfen, ob sich durch das Abwerfen der Miele-Altlasten das Caching umgehen lässt.
1. **Erwartung:** Die Software-Version springt auf `1.0.0-bert`.
2. **Erwartung:** Der Gerätename wird im Onboarding sauber als `BertsMatterSchmiede` angezeigt.

---

## 3. Warum ist NICHTS passiert?

### Punkt A: Der "TEST_VENDOR" bleibt Schicksal
Wie im Guide (Abschnitt 8) bereits vermutet: Für die Test-VID `0xFFF1` erzwingt Google Home in den **Technischen Informationen** (Einstellungsmenü) *immer* die Platzhalter `TEST_VENDOR` und `TEST_PRODUCT`. Das ist tief in der App hardcodiert und lässt sich nicht per Firmware überschreiben.

### Punkt B: Google Homes Cloud-Gedächtnis (Version `2.3.7`)
Dass weiterhin die Version `2.3.7` und der Name `TEST_PRODUCT (2)` auftauchen, beweist ein extremes Cloud-Caching:
* Google Home erkennt die Kombination aus VID `0xFFF1` und PID `0x8001`.
* Statt die Attribute (`1.0.0-bert`) frisch vom Gerät via Bluetooth/Matter auszulesen, zieht Google die alten Metadaten aus der Cloud-Historie deines Google-Accounts.
* Da bereits ein Gerät mit diesen IDs existierte, wird das neue einfach als Kopie `(2)` angelegt.

---

## 4. Fazit für externe Experten
Das Problem liegt **nicht** im ESP32-Code. Die Logs bestätigen den korrekten Output. Das Problem ist das Ökosystem von Google Home:
1. **Test-VIDs** (`0xFFF1`) werden in den Details niemals echtes Branding anzeigen.
2. **Einmal gecached, immer gecached:** Das cloudseitige Löschen der Geräteeigenschaften bei Google ist für Entwickler extrem unzuverlässig.

**Einzige Lösung für echtes Branding:** Der offizielle Weg über eine eigene CSA-VID (Kostenpflichtig).
