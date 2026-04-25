<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" style="height:64px;margin-right:32px"/>

# Wenn gewünscht, kann daraus als Nächstes ein technisches Entwickler-Memo in Stichpunkten oder eine saubere Mapping-Tabelle für den Code erstellt werden.

das wünsche ich mir

Gern — hier ist ein **technisches Entwickler-Memo** plus eine **saubere Mapping-Tabelle** für den Code, fokussiert auf die Spülmaschine als Matter-Endpoint.[^1][^2]

## Entwickler-Memo

- Die Bridge soll Miele-Cloud-Zustände nicht direkt, sondern über ein internes State-Modell auf Matter abbilden.
- `On/Off` ist ein echter Power-State und **nicht** Programmlaufsteuerung.
- `Operational State` ist für den Gerätezustand während des Zyklus zuständig.
- `Dishwasher Mode` ist für das gewählte Programm bzw. die Betriebsart zuständig.
- `remaining_time_mins` sollte gepuffert und als Countdown/Restlaufzeit exponiert werden.
- Google Home ist für Test-VIDs kein zuverlässiger Referenzpunkt für technische Detailwerte; für Protokoll- und Attribute-Validierung sollte zusätzlich `chip-tool` oder ein zweiter Controller genutzt werden.[^3][^2]
- In esp-matter wird die App-Logik typischerweise über Delegate-/Callback-Logik an Cluster gebunden.[^1]


## Mapping-Tabelle

| Miele-Feld | Bedeutung | Matter-Ziel | Implementierungshinweis |
| :-- | :-- | :-- | :-- |
| `is_on` | echtes An/Aus | `OnOff` Cluster | Direkt auf `OnOff::Attributes::OnOff` mappen [^4] |
| `status_id` | Betriebszustand | `Operational State` Cluster | In `OperationalStateEnum` übersetzen, z. B. running, paused, stopped [^2][^5] |
| `program_id` | gewähltes Programm | `Dishwasher Mode` Cluster | Auf validierte Dishwasher-Modes mappen, nicht mit Power-State vermischen [^2][^6] |
| `remaining_time_mins` | Restlaufzeit | `Operational State` / Countdown-Time | Intern puffern, als Countdown-Attribut ausgeben [^2][^7] |
| `device_id` | Geräteidentität | Matter Endpoint / Node-Metadaten | Für Zuordnung, Logging und Persistenz nutzen |

## Code-Regeln

### 1) Power-State separat halten

`OnOff` soll nur den echten An-/Aus-Zustand abbilden. Wenn die Maschine eingeschaltet ist, aber kein Programm läuft, bleibt `OnOff = true` und `Operational State = idle/stopped`.[^4][^2]

### 2) Zustandsmaschine statt Direktmapping

`status_id` sollte nicht direkt in UI-Werte geschrieben werden. Besser:

- Rohwert aus der Cloud lesen
- in internes Enum übersetzen
- daraus Matter-Cluster updaten


### 3) Restlaufzeit puffern

`remaining_time_mins` nie ungefiltert anzeigen. Nutze:

- letzten gültigen Wert
- Zeitstempel
- Fallback auf `nullable` oder `unknown`, wenn der Wert zu alt ist


### 4) Programme validieren

`program_id` nur auf bekannte Programme mappen. Unbekannte Programme sollten nicht abstürzen, sondern als `unknown` oder generischer Standardmodus behandelt werden.[^6]

## Empfohlene Cluster-Nutzung

- **Endpoint 0:** Root/Commissioning/Basic Information
- **Endpoint 1:** Dishwasher Device Type
- **Endpoint 1:** `On/Off`
- **Endpoint 1:** `Operational State`
- **Endpoint 1:** `Dishwasher Mode`


## Offene Fachfragen für den nächsten Implementierungsschritt

1. Welche konkreten `status_id`-Werte liefert die Miele-API?
2. Welche Programme sollen zuerst in `Dishwasher Mode` unterstützt werden?
3. Soll `remaining_time_mins` in Matter als reiner Countdown oder zusätzlich als Fortschrittslogik dargestellt werden?
4. Welche Zustände sollen bei Cloud-Ausfall als letzter bekannter Wert gepuffert werden?

## Empfehlung für die nächste Code-Iteration

- Erst die interne State-Struktur definieren.
- Dann die `status_id`- und `program_id`-Mappings festlegen.
- Danach `OnOff` und `Operational State` getrennt anbinden.
- Zuletzt `remaining_time_mins` als gepuffertes Attribut ergänzen.

Wenn du möchtest, kann ich im nächsten Schritt direkt eine **C++-Struktur `dishwasher_state_t` plus Mapping-Funktionen** für `status_id`, `program_id`, `is_on` und `remaining_time_mins` formulieren.
<span style="display:none">[^10][^11][^12][^13][^14][^15][^8][^9]</span>

<div align="center">⁂</div>

[^1]: https://docs.espressif.com/projects/esp-matter/en/latest/esp32/app_guide.html

[^2]: https://tomasmcguinness.com/2025/06/27/matter-building-a-tiny-dishwasher-with-an-esp32/

[^3]: https://project-chip.github.io/connectedhomeip-doc/development_controllers/chip-tool/chip_tool_guide.html

[^4]: https://docs.silabs.com/matter/2.8.1/matter-application-development/matter-application-cluster-logic

[^5]: https://community.home-assistant.io/t/support-for-matter-operationalstate-cluster/751980

[^6]: https://forum.aqara.com/t/improve-support-for-matter-dishwasher-in-aqara-app/62737

[^7]: https://esp32.com/viewtopic.php?t=45917

[^8]: https://github.com/tomasmcguinness/matter-esp32-tiny-dishwasher

[^9]: https://docs.espressif.com/projects/esp-matter/en/latest/esp32c6/esp-matter-en-master-esp32c6.pdf

[^10]: https://tomasmcguinness.com/2025/07/26/matter-tiny-dishwasher-adding-energy-forecast/

[^11]: https://www.reddit.com/r/esp32/comments/1lb1mzp/understanding_operationalstate_in_espmatter/

[^12]: https://csa-iot.org/wp-content/uploads/2023/10/Matter-1.2-Application-Cluster-Specification.pdf

[^13]: https://developer.apple.com/documentation/Matter

[^14]: https://docs.nordicsemi.com/bundle/ncs-2.9.2/page/nrf/protocols/matter/overview/data_model.html

[^15]: https://www.scribd.com/document/778266504/Matter-Application-Cluster-Specification

