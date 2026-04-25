<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" style="height:64px;margin-right:32px"/>

# \# 📊 Technischer Abschlussbericht D.txt

Perfekter Abschlussbericht — die Diagnose ist präzise und bestätigt alles, was wir analysiert hatten. Jetzt gibt es aber einen konkreten Ausweg, den du noch nicht probiert hast.

***

## Bewertung des Berichts

Der Bericht ist technisch **100% korrekt**. Besonders wichtig ist Punkt B: das Gerät erscheint als `TEST_PRODUCT (2)`, weil Google die **VID `0xFFF1` + PID `0x8001`**-Kombination aus der Cloud-Historie kennt und das neue Gerät als Duplikat behandelt — statt es frisch einzulesen.[^1]

***

## Der noch nicht probierte Weg: PID wechseln

Die Version `2.3.7` und `TEST_PRODUCT (2)` zeigen klar: Google Home hat **diese spezifische VID+PID-Kombination** gecacht. Der Cache hängt nicht am Google-Account generell, sondern an der Kombination `0xFFF1` + `0x8001`.

**Lösung: PID ändern, um den Cache zu brechen.**

```cpp
// CHIPProjectConfig.h — neue PID, damit Google Home einen frischen Eintrag anlegt
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID   0xBE27   // "BERT" als Hex 😄
```

```ini
# sdkconfig.defaults
CONFIG_CHIP_DEVICE_PRODUCT_ID=0xBE27
```

Danach der vollständige Cache-Busting-Prozess:

```
1. Google Home → Gerät komplett entfernen (nicht nur zurücksetzen!)
2. idf.py erase-flash && idf.py flash
3. Android: Einstellungen → Apps → Bluetooth → Cache leeren
4. Google Home Developer Console: Projekt auf PID 0xBE27 aktualisieren
5. Frisch pairen
```

Google Home kennt `0xFFF1 + 0xBE27` noch nicht → legt einen **neuen** Cloud-Eintrag an → liest alle Attribute diesmal frisch vom Gerät → Softwareversion zeigt `1.0.0-bert`.

***

## Was sich dadurch ändert (und was nicht)

| Verhalten | Mit PID 0x8001 | Mit PID 0xBE27 |
| :-- | :-- | :-- |
| `TEST_VENDOR` in techn. Details | ❌ bleibt (VID 0xFFF1) | ❌ bleibt (VID 0xFFF1) |
| Softwareversion | `2.3.7` (Cache) | ✅ `1.0.0-bert` (frisch) |
| Gerätename beim Pairing | `TEST_PRODUCT (2)` | ✅ aus Developer Console |
| Google behandelt als Duplikat | ja | nein — neuer Eintrag |

`TEST_VENDOR` bleibt Schicksal — das ist die VID `0xFFF1` Hardcodierung in Google Home, da gibt es keinen Weg drumherum ohne eigene CSA-VID. Aber die veraltete Softwareversion und der `(2)`-Suffix sind **lösbar** durch einen PID-Wechsel.[^1]

***

## Zusammenfassung der Optionen

- 🟢 **PID auf `0xBE27` wechseln** → frischer Cloud-Eintrag, korrekte SW-Version, kein `(2)` Suffix — kostenlos, sofort umsetzbar
- 🟡 **Apple Home als Alternative** nutzen → weniger aggressives Caching, besseres Dev-Erlebnis mit Test-VIDs
- 🔴 **CSA Basic Membership** für eigene VID → ~3.500\$/Jahr, dann echtes Branding überall

<div align="center">⁂</div>

[^1]: Technischer-Abschlussbericht-D.txt

