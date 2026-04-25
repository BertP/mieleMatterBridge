# 📊 Technischer Bericht: Analyse des "TEST_VENDOR"-Verhaltens in Google Home

**Datum:** 25.04.2026  
**Status:** Analyse für externe Expertise  

---

## 1. Problembeschreibung
Trotz korrekter Konfiguration der Firmware auf Version `2.3.9` und korrekter IDs (`VID=0xFFF1`, `PID=0x8001`) sowie der Hinterlegung des Miele-Brandings in der Google Home Developer Console, zeigt die Google Home App in den Geräteeinstellungen ("Technische Informationen") weiterhin folgende Werte an:
* **Hersteller:** `TEST_VENDOR`
* **Modell:** `TEST_PRODUCT`
* **Softwareversion:** `2.3.7` (statt `2.3.9`)

---

## 2. Ursachenanalyse

### Ursache A: Die Test-VID `0xFFF1` Einschränkung
Die Google Home App besitzt eine **globale Hardcodierung** für den CSA-Testbereich (`0xFFF1` bis `0xFFF4`).
* **Verhalten:** Die in der Google Home Developer Console hinterlegten Branding-Daten (Name, Logo) greifen **nur** während des Pairing-Prozesses (Onboarding).
* **Konsequenz:** Sobald das Gerät erfolgreich gekoppelt ist, fällt die Detail-Ansicht der Google Home App hart auf die Standard-Platzhalter `TEST_VENDOR` und `TEST_PRODUCT` zurück. Echtes Branding in den Einstellungen ist ausschließlich für produktive, von der CSA signierte VIDs vorgesehen.

### Ursache B: Persistenter Cloud-Cache (Softwareversion `2.3.7`)
Die Anzeige der veralteten Softwareversion `2.3.7` belegt, dass Google Home die Gerätemetadaten cloudseitig cached.
* Selbst ein `idf.py erase-flash` auf dem ESP32 löscht diesen Cache nicht, da Google die Geräteeigenschaften an die Fabric-ID bzw. den Cloud-Eintrag koppelt.

---

## 3. Log-Beweisführung (Firmware vs. App)

Die folgenden Log-Auszüge belegen zweifelsfrei, dass die Firmware auf dem ESP32 die korrekten Werte ausspielt:

### Boot & Advertising (`VID=0xFFF1`, `PID=0x8001`)
```text
I (1490) chip[DIS]: Advertise commission parameter vendorID=65521 productID=32769 discriminator=3840/15 cm=1 cp=0 jf=0
I (1500) chip[DIS]: CHIP minimal mDNS configured as 'Commissionable node device'
I (1540) chip[DL]: Configuring CHIPoBLE advertising (interval 25 ms, connectable)
I (1570) chip[DL]: CHIPoBLE advertising started
```
*(Hinweis: 65521 = 0xFFF1, 32769 = 0x8001)*

### Cluster-Attribute (`Version 2.3.9`)
Der ESP32 schreibt die korrekten Daten in das Basic Information Cluster (Endpoint 0):
```text
I (2080) esp_matter_attribute: ********** W : Endpoint 0x0000's Cluster 0x00000028's Attribute 0x00000002 is 65521 **********
I (2080) esp_matter_attribute: ********** W : Endpoint 0x0000's Cluster 0x00000028's Attribute 0x00000004 is 32769 **********
I (2100) esp_matter_attribute: ********** W : Endpoint 0x0000's Cluster 0x00000028's Attribute 0x00000001 is Miele & Cie. KG **********
I (2140) esp_matter_attribute: ********** W : Endpoint 0x0000's Cluster 0x00000028's Attribute 0x0000000A is 2.3.9 **********
```

---

## 4. Fazit & Handlungsoptionen für Experten
Die Firmware arbeitet spezifikationskonform. Das Verhalten ist eine Designentscheidung des Google Home Ökosystems. 

**Lösungswege:**
1. **Entwicklungsphase:** Das Verhalten muss als "Kosmetikfehler der Test-VID" akzeptiert werden.
2. **Produktion:** Beantragung einer echten VID (`0x120E`) bei der Connectivity Standards Alliance (CSA) und Nutzung von offiziellen PAA/PAI/DAC-Zertifikaten.
