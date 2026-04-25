# 💡 FuckUp Night: Wie wir versuchten, Miele ins Matter-Universum zu zwingen und an Googles Cloud-Gedächtnis zerschellten

**Projekt:** Miele Matter Bridge (ESP32-S3)  
**Speaker:** Bert  
**Motto:** "Einfach mal die Vendor ID fälschen? Was soll schon schiefgehen!"

---

## 🎬 Akt 1: Die Hybris am Anfang
Es fing alles so unschuldig an. Ein ESP32-S3 DevKit, ein kleines OLED-Display und der Traum, Miele-Haushaltsgeräte nativ in Google Home via Matter einzubinden. Nicht als generisches Bastelprojekt, sondern mit Stil: Echtes **"Miele & Cie. KG"**-Branding in der App. 

Wir dachten: "Matter ist Open Source, wir haben den ESP-Matter SDK-Code — wir sind die Herren der Attribute!"

## 🌋 Akt 2: Die Windows-Build-Hölle & der WSL-Exodus
Die erste Lektion im Matter-Universum: Wer versucht, den `connectedhomeip`-Stack unter Windows nativ zu kompilieren, verliert seinen Verstand. 
* 15 bis 20 Minuten pro Build.
* Ständige Pfadlängen-Fehler.
* Python-Umgebungen, die sich gegenseitig auffraßen.

**Der Wendepunkt:** Ein kompletter Systemabsturz. Der komplette bisherige Code war weg.  
**Der Schwenk:** Flucht nach WSL2 (Ubuntu). Plötzlich dauerte ein Build nur noch Sekunden. Doch die Performance-Sonne blendete uns für die dunklen Wolken, die sich am Google-Horizont zusammenzogen.

## 🛑 Akt 3: Die Wand namens `TEST_VENDOR`
Wir passten `CHIPProjectConfig.h` an, überschrieben die Cluster-Attribute zur Laufzeit im `main.cpp`. Der ESP32 schrie förmlich über die serielle Schnittstelle: *"Ich bin eine Miele Bridge!"*. 

Wir starteten das Pairing in Google Home. Und was zeigte die App nach 10 Minuten nervenzerfetzendem Warten in den Gerätedetails?  
👉 **`TEST_VENDOR`**  
👉 **`TEST_PRODUCT`**  

Wir flashten neu. Wir machten `idf.py erase-flash`. Wir drehten Bluetooth am Handy ab. Wir löschten den NVS. Nichts half. Google Home ignorierte unsere Firmware-Befehle und beharrte arrogant auf den Standard-Testplatzhaltern.

## 🧠 Akt 4: Der KI-Schlag in die Magengrube (Die Perplexity/Claude Expertise)
Nachdem wir uns im Kreis gedreht hatten, konsultierten wir die externe Expertise (Perplexity / Claude Sonnet). Die Diagnose traf uns wie ein nasses Handtuch:
1. **Die Google-Hardcodierung:** Google Home hat für die offizielle Test-VID `0xFFF1` eine felsenfeste UI-Hardcodierung. Es ist der App völlig egal, was im Chip steht. Test-VID = `TEST_VENDOR`. Punkt.
2. **Das unendliche Cloud-Gedächtnis:** Google cached Geräte-IDs (`VID` + `PID`) gnadenlos im Google Account. Einmal als `2.3.7` registriert, bleibt das Gerät cloudseitig auf dieser Version, selbst wenn der Chip längst neu bespielt wurde. Deshalb hieß unser zweiter Versuch am Ende traurig: `TEST_PRODUCT (2)`.

---

## 🏁 Das FuckUp-Fazit: Gemini vs. Claude

Beim Reflektieren über das Scheitern zeigt sich auch der Unterschied in der "KI-Denke":

* **Was Gemini Flash wusste/tat:** Extrem stark im pragmatischen "Machen". Hat fehlerfreien C++ Code generiert, Makefiles gebogen und die Toolchain in Rekordzeit zum Laufen gebracht. Gemini glaubte an den Erfolg durch Code.
* **Was Claude Sonnet 4.6 wusste:** Besaß das tiefe, schmutzige Nischenwissen über die **Ökosystem-Politik**. Sonnet wusste, dass Matter kein reines Software-Protokoll ist, sondern ein geschlossenes Zertifizierungssystem der Connectivity Standards Alliance (CSA). Code allein besiegt die Cloud-Caching-Mechanismen von Google nicht.

**Die ultimative Erkenntnis:** Du kannst den saubersten Code der Welt schreiben — gegen die Cloud-Richtlinien der Tech-Giganten hast du als kleiner Entwickler ohne 3.500 $/Jahr für eine offizielle CSA-Mitgliedschaft keine Chance. 

*Applaus, Vorhang, Bier.* 🍻
