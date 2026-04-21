# Matter Bridge Mapping: Dishwasher ↔ Miele 3rd Party API

## Ziel
Dieses Dokument beschreibt ein praxisnahes Mapping zwischen einem **Matter Dishwasher Device Type** und der **Miele 3rd Party API** – in beide Richtungen.

Der Fokus liegt auf den in Matter 1.5.1 für Geschirrspüler relevanten Clustern:
- `Identify`
- `On/Off`
- `Temperature Control`
- `Dishwasher Mode`
- `Dishwasher Alarm`
- `Operational State`

Zusätzlich werden die Miele-API-Bausteine berücksichtigt, die laut öffentlicher Dokumentation zur Verfügung stehen:
- Geräteübersicht
- Geräte-Identifikation
- Gerätezustand
- Aktionen
- Eventing via SSE

## Annahmen und Leitplanken

1. **Matter ist die führende lokale Steuer- und Zustandsabstraktion.** Die Bridge übersetzt Matter-Cluster in Miele-API-Aufrufe und umgekehrt.
2. **Die Miele API ist generisch modelliert.** Nicht unterstützte Eigenschaften liefern `null` oder `-32768`.
3. **Zustandsänderungen sollten bevorzugt über Eventing (SSE) statt Polling verarbeitet werden.**
4. **Der Matter-Dishwasher benötigt den `Operational State` Cluster als Pflichtcluster.**
5. **`On/Off` hat für Matter-Dishwasher die `DeadFrontBehavior`-Anforderung.**
6. **Die genaue Feldstruktur aus dem Miele Swagger sollte in einer Implementierung noch gegen die offizielle API geprüft werden.** Dieses Dokument beschreibt daher ein robustes Integrationsmodell, nicht jede einzelne JSON-Property im Detail.

## Relevante Matter-Referenzen

Der Matter Dishwasher Device Type (`0x0075`) verlangt auf dem Endpoint folgende Cluster:

| Cluster | ID | Typ | Conformance |
|---|---:|---|---|
| Identify | `0x0003` | Server | O |
| On/Off | `0x0006` | Server | O |
| Temperature Control | `0x0056` | Server | O |
| Dishwasher Mode | `0x0059` | Server | O |
| Dishwasher Alarm | `0x005D` | Server | O |
| Operational State | `0x0060` | Server | M |

Zusatzregeln:
- Ein Dishwasher Cycle ist eine Kombination aus Modus und Temperatur.
- `On/Off` muss die `DeadFrontBehavior`-Logik unterstützen.
- Im `Dead Front` Zustand sollen bestimmte Attribute best-effort Werte liefern, z. B. `OperationalState = Stopped` und `OperationalError = No Error`.

## Relevante Miele-API-Bausteine

Aus der öffentlichen Miele-Dokumentation sind derzeit sicher bekannt:
- `GET /devices` liefert eine Liste registrierter Geräte.
- `GET /devices/<deviceId>/ident` liefert Identifikationsdaten.
- `GET /devices/<deviceId>/state` liefert den aktuellen Gerätezustand.
- `GET /devices/<deviceId>/actions` liefert die aktuell möglichen Aktionen.
- `PUT /devices/<deviceId>/actions` führt eine Aktion aus.
- Eventing erfolgt via SSE mit den Eventtypen `PING`, `IDENT` und `ACTION`.

Bekannt dokumentierte Identifikationsfelder:
- `fabNumber` als eindeutige Gerätekennung und zugleich `deviceId`
- `type` als Basistyp
- `techType` als Katalogtyp

## Empfohlenes Bridge-Design

Die Bridge sollte drei interne Zustandsbereiche führen:

1. **Identität**
   - Miele Geräteliste und Identitätsdaten
   - Matter Descriptor/Basic Information/Identify

2. **Bedienung und Laufzustand**
   - Matter `On/Off`
   - Matter `Operational State`
   - Matter `Dishwasher Mode`
   - Matter `Temperature Control`

3. **Fehler und Warnungen**
   - Matter `Dishwasher Alarm`
   - Miele Fehler-/Statusdaten aus `state`
   - Miele `ACTION`-Events für verfügbare Aktionen

## Top-Level Mapping

### 1) Gerätemodell

| Matter | Miele | Richtung | Transformation |
|---|---|---|---|
| Dishwasher Device Type `0x0075` | Dishwasher Appliance Type | Matter ⇄ Miele | 1:1 auf Geräteklasse |
| Descriptor / PartsList / TagList | Miele Geräteliste + Geräte-Metadaten | Matter ⇐ Miele | Bridge erzeugt Matter-Endpoint-Topologie |
| Basic Information / Node identity | `ident.fabNumber`, `ident.type`, `ident.techType` | Matter ⇐ Miele | Fabric-/Node-spezifische Identität + Appliance-Identity |

### 2) Zustandsabruf

| Matter Cluster | Miele Quelle | Richtung | Transformation |
|---|---|---|---|
| Identify | `ident` | Matter ⇐ Miele | Name/Label/Seriennummer für Erkennung |
| On/Off | `state` | Matter ⇐ Miele | Power/enable-status, ableitbar aus Betriebszustand |
| Temperature Control | `state` | Matter ⇐ Miele | Temperatur bzw. Temperaturstufe, falls unterstützt |
| Dishwasher Mode | `state` + `actions` | Matter ⇐ Miele | Programmlogik und aktuell gewählter Modus |
| Dishwasher Alarm | `state` | Matter ⇐ Miele | Fehler-/Warnbits aus Status/Fehlerindikatoren |
| Operational State | `state` | Matter ⇐ Miele | Laufzustand, Restzeit, Phase, Error |

### 3) Steuerung

| Matter Befehl / Attribut | Miele Aktion | Richtung | Transformation |
|---|---|---|---|
| `OnOff.On` | `PUT /actions` mit Start/Resume-ähnlicher Aktion | Matter ⇒ Miele | Startet/reaktiviert einen Zyklus, sofern erlaubt |
| `OnOff.Off` | `PUT /actions` mit Stop/Cancel-ähnlicher Aktion | Matter ⇒ Miele | Stoppt Gerät / versetzt in Dead Front |
| `Dishwasher Mode.ChangeToMode` | `PUT /actions` mit Programmwahl | Matter ⇒ Miele | Mappt Matter-Mode auf Miele-Programm |
| `Temperature Control.SetTemperature` | `PUT /actions` oder programmbezogene Temperaturwahl | Matter ⇒ Miele | Je nach Miele-Kapazität Temperatur oder Level setzen |
| `Operational State.Start` | `PUT /actions` mit Start | Matter ⇒ Miele | Zyklusstart |
| `Operational State.Stop` | `PUT /actions` mit Stop | Matter ⇒ Miele | Zyklusstopp |
| `Operational State.Pause/Resume` | `PUT /actions` mit Pause/Resume, falls verfügbar | Matter ⇒ Miele | Nur wenn Miele die Aktion im aktuellen Zustand erlaubt |

## Cluster-Mapping im Detail

## 1. Identify Cluster (`0x0003`)

### Matter → Miele
- Matter `Identify` dient primär zur lokalen visuellen/akustischen Identifikation.
- Auf Miele-Seite gibt es dafür typischerweise keine direkte Funktionsentsprechung.
- Sinnvolle Bridge-Umsetzung:
  - Name/Label des Geräts aus `ident.type` und `ident.techType` ableiten.
  - Falls die Bridge eine lokale Anzeige besitzt, kann sie eine Identifikationssequenz simulieren.

### Miele → Matter
- `fabNumber` kann als stabile Gerätekennung genutzt werden.
- `type` und `techType` können für den Matter-Raum als benutzerlesbares Label dienen.

### Empfehlung
- `Identify` ist in der Bridge eher ein **lokaler Komfort-Cluster** als eine API-Aktion.

## 2. On/Off Cluster (`0x0006`)

Matter verlangt für Dishwasher den `DeadFrontBehavior`-Mechanismus. `Off` bedeutet daher nicht „elektrisch stromlos“, sondern der logische Zustand, in dem Bedien- und Zustandswerte best-effort bzw. nicht aktiv sind.

### Matter → Miele
| Matter | Miele | Hinweis |
|---|---|---|
| `OnOff = false` / `Off` | Stop/Cancel/Standby-artige Aktion | Versetzt das Gerät in einen nicht aktiven Zustand |
| `OnOff = true` / `On` | Wake/Ready/Enable-artige Aktion | Verläßt den Dead-Front-Zustand |

### Miele → Matter
| Miele Zustand | Matter `OnOff` | Hinweis |
|---|---|---|
| Gerät bereit / betriebsbereit | `true` | User kann einen Zyklus auswählen oder starten |
| Gerät inaktiver / gesperrter Zustand | `false` | Dead Front bzw. nicht bedienbar |

### Implementierungsnote
- Bei `Off` sollten die übrigen clusterspezifischen Werte auf best-effort Werte abgebildet werden.
- Matter erlaubt in diesem Zustand beim Dishwasher best-effort Werte wie `OperationalState = Stopped` und `OperationalError = No Error`.

## 3. Temperature Control Cluster (`0x0056`)

Matter definiert hier zwei mögliche Modellierungsarten:
- `TemperatureNumber` für tatsächliche Temperaturwerte
- `TemperatureLevel` für qualitative Stufen

Für Geschirrspüler ist in der Praxis oft die Miele-Programmlogik relevanter als eine frei einstellbare Zieltemperatur. Deshalb sollte die Bridge die Temperatur als **programmgebundene Eigenschaft** behandeln.

### Empfohlene Abbildung

| Matter | Miele | Richtung | Transformation |
|---|---|---|---|
| `TemperatureNumber` | Programmtemperatur, falls exponiert | Matter ⇐ Miele | Celsius-Wert, wenn Miele einen konkreten Temperaturwert liefert |
| `TemperatureLevel` | Programmniveau / Intensität | Matter ⇐ Miele | Diskrete Stufen wie niedrig/mittel/hoch oder programmspezifische Stufen |
| `SetTemperature` | Temperatur-/Programmwahl | Matter ⇒ Miele | Auswahl der nächsten passenden Programmbedingung |

### Praktische Regel
- Wenn Miele einen echten Temperaturwert liefert, nutze `TemperatureNumber`.
- Wenn Miele nur Programmnamen oder Stufen liefert, nutze `TemperatureLevel` und mappe die Stufe auf ein Matter-kompatibles Niveau.

## 4. Dishwasher Mode Cluster (`0x0059`)

Dieser Cluster ist der wichtigste Match für das eigentliche Waschprogramm.

Matter definiert die aus Sicht des Dishwashers relevanten Tags:
- `Normal` (`0x4000`)
- `Heavy` (`0x4001`)
- `Light` (`0x4002`)

Außerdem fordert Matter, dass mindestens ein Eintrag in `SupportedModes` den `Normal`-Tag enthält.

### Empfohlene Zuordnung von Miele-Programmen zu Matter-Modes

| Miele Programmtyp / Programmname | Matter Mode | Begründung |
|---|---|---|
| Auto / Standard / Normal | `Normal` | Alltagsprogramm |
| Intensiv / Pots & Pans / Heavy Duty | `Heavy` | Stark verschmutztes Geschirr |
| Schnell / Kurz / Gentle / Delicate | `Light` | Schonender oder leichter Waschgang |

### Mode-Set Beispiel

| Matter `SupportedModes` Entry | Miele Äquivalent |
|---|---|
| Label: `Auto`, Tag: `Normal` | Standardprogramm |
| Label: `Intensive`, Tag: `Heavy` | Intensivprogramm |
| Label: `Quick`, Tag: `Light` | Kurzprogramm |

### Matter → Miele
| Matter | Miele | Richtung | Transformation |
|---|---|---|---|
| `CurrentMode` | aktuelles Waschprogramm | Matter ⇐ Miele | Programminternes Mapping |
| `ChangeToMode(NewMode)` | Programmauswahl | Matter ⇒ Miele | Startet die Programmauswahl vor Zyklusstart |
| `StartUpMode` | bevorzugtes Startprogramm | Matter ⇐ Miele | Optional, wenn Miele einen Default liefert |
| `OnMode` | Aktivierungsmodus | Matter ⇐ Miele | Nur relevant, wenn Miele einen separaten aktivierten Zustand kennt |

### Wichtige Einschränkung
- Matter schreibt vor, dass `StartUpMode` im Dishwasher Mode Cluster nicht verwendet werden soll, sofern durch die Device-Type-Regeln disallowed.
- Für die Bridge sollte daher `CurrentMode` und `SupportedModes` im Vordergrund stehen.

## 5. Dishwasher Alarm Cluster (`0x005D`)

Der Matter Dishwasher Alarm Cluster enthält Alarmbits, die sich gut auf Miele-Fehler und Warnungen abbilden lassen.

### Matter Alarm-Bits
- `InflowError`
- `DrainError`
- `DoorError`
- `TempTooLow`
- `TempTooHigh`
- `WaterLevelError`

### Empfohlene Miele → Matter Zuordnung

| Matter Alarmbit | Miele Signal / Zustand | Richtung | Kommentar |
|---|---|---|---|
| `InflowError` | Zulauf-/Wasserzulauf-Fehler | Matter ⇐ Miele | Wenn Miele einen entsprechenden Fehler meldet |
| `DrainError` | Ablauffehler / Pumpen-/Abflussfehler | Matter ⇐ Miele | Wenn Abfluss blockiert ist |
| `DoorError` | Tür offen / Türverriegelung / Türkontaktfehler | Matter ⇐ Miele | Sehr naheliegend bei Gerätesperre |
| `TempTooLow` | Aufheizung zu langsam / Solltemperatur nicht erreicht | Matter ⇐ Miele | Nur falls in Miele-Status erkennbar |
| `TempTooHigh` | Temperaturüberschreitung / Sicherheitsfehler | Matter ⇐ Miele | Nur falls vorhanden |
| `WaterLevelError` | Füllstand/Wasserstand unplausibel | Matter ⇐ Miele | Falls Miele einen entsprechenden Fehler exponiert |

### Best practice
- Wenn Miele nur eine allgemeine Fehlerklasse liefert, mappe diese zuerst auf `Operational State = Error` und setze zusätzlich das passendste Alarmbit.
- Wenn mehrere Fehler gleichzeitig anliegen, können mehrere Bits gesetzt werden.

## 6. Operational State Cluster (`0x0060`)

Matter verlangt für den Dishwasher den `Operational State` Cluster als Pflichtcluster.

### Matter-Standardzustände
- `Stopped`
- `Running`
- `Paused`
- `Error`

### Empfohlene Miele ↔ Matter Zustandsabbildung

| Miele Zustand / Status | Matter `OperationalState` | Anmerkung |
|---|---|---|
| Gerät bereit, kein Zyklus aktiv | `Stopped` | Standard-Idle/Ready-nah |
| Waschzyklus läuft | `Running` | Zyklus aktiv |
| Zyklus angehalten | `Paused` | Nur wenn Miele dies tatsächlich unterstützt |
| Fehler / Störung | `Error` | Zusätzlich `OperationalError` belegen |

### Phasen- und Zeitabbildung

| Matter Attribut | Miele Quelle | Richtung | Transformation |
|---|---|---|---|
| `PhaseList` | Programmschritt(e) aus Zustand | Matter ⇐ Miele | Optional, wenn Miele Phasen liefert |
| `CurrentPhase` | aktueller Programmschritt | Matter ⇐ Miele | Index in der Phase-Liste |
| `CountdownTime` | Restzeit | Matter ⇐ Miele | Sekunden bis Ende |
| `OperationalError` | Fehlerstatus | Matter ⇐ Miele | `No Error` wenn kein Fehler vorliegt |

### Best-effort-Werte im Dead-Front-Zustand
Falls das Gerät über `OnOff = false` in den Dead-Front-Zustand wechselt, sollten mindestens diese Werte geliefert werden:
- `Dishwasher Mode.CurrentMode = MS` (manufacturer specific, wenn der Zustand nicht sicher sinnvoll ist)
- `Temperature Control = MS`
- `OperationalState.PhaseList = null`
- `OperationalState.CurrentPhase = null`
- `OperationalState.CountdownTime = null`
- `OperationalState.OperationalState = Stopped`
- `OperationalState.OperationalError = No Error`

## Bidirektionale Mapping-Tabelle

| Matter | Miele | Richtung | Kommentar |
|---|---|---|---|
| `SupportedModes` | verfügbare Programme aus `actions` | Matter ⇐ Miele | Bridge erzeugt die Liste der unterstützten Programme |
| `CurrentMode` | aktives Programm | Matter ⇐ Miele | State-Driven Mapping |
| `ChangeToMode` | Programmwahl | Matter ⇒ Miele | Vor Zyklusstart |
| `TemperatureSetpoint` / Temperaturwahl | temperaturbezogene Programmauswahl | Matter ⇒ Miele | Nur falls das Modell es anbietet |
| `OnOff = true` | Gerät aktivieren | Matter ⇒ Miele | Aus dem deaktivierten Zustand heraus |
| `OnOff = false` | Gerät stoppen/deaktivieren | Matter ⇒ Miele | Dead Front |
| `OperationalState = Running` | Zyklus läuft | Matter ⇐ Miele | Aus `state` ableiten |
| `OperationalState = Error` | Fehler | Matter ⇐ Miele | Fehlerdetails zusätzlich in Alarm abbilden |
| `CountdownTime` | Restzeit | Matter ⇐ Miele | Sekunden |
| `OperationalError` | Fehlercode | Matter ⇐ Miele | `No Error` bei normalem Betrieb |

## Vorschlag für interne Bridge-Regeln

### 1. Programm-Mapping
- Miele-Programme werden in eine kleine, stabile Matter-Mode-Menge überführt.
- Unbekannte Programme können als `Manufacturer Specific`-Modus behandelt werden, wenn ein genauer Standard-Match fehlt.

### 2. Temperatur-Mapping
- Falls die Miele API einen numerischen Wert liefert, verwende `TemperatureNumber`.
- Falls nur Programmnamen oder Stufen vorliegen, nutze `TemperatureLevel`.

### 3. Fehler-Mapping
- Zustandsfehler aus Miele zuerst in `Operational State = Error` übersetzen.
- Danach die passenden Alarmbits in `Dishwasher Alarm` setzen.

### 4. Eventing
- SSE-`IDENT` aktualisiert Identität und Zustand.
- SSE-`ACTION` aktualisiert mögliche Aktionen und damit die Matter-Fähigkeiten.
- SSE-`PING` wird nur für Verbindungsüberwachung verwendet.

## Open Points für die finale Implementierung

Diese Punkte sollten gegen den vollständigen Miele Swagger validiert werden:
- Exakte JSON-Feldnamen in `state`
- Liste der Programmaktionen in `actions`
- Konkrete Befehle für Start / Pause / Resume / Stop
- Konkrete Temperaturdarstellung pro Geschirrspüler-Modell
- Fehlercodes und ihre Granularität

## Empfohlene Implementierungsreihenfolge

1. **Device Discovery und Identität**
   - `GET /devices`
   - `GET /devices/<deviceId>/ident`

2. **State Sync**
   - `GET /devices/<deviceId>/state`
   - SSE abonnieren

3. **Matter Core Clusters**
   - `On/Off`
   - `Operational State`
   - `Dishwasher Mode`

4. **Erweiterung**
   - `Temperature Control`
   - `Dishwasher Alarm`

5. **Fehlerrobustheit**
   - Dead Front
   - Miele `null` / `-32768`
   - Nicht unterstützte Aktionen

## Kompakte Zusammenfassung

Die sauberste Bridge-Abbildung ist:
- **Miele-Programmwahl → Matter `Dishwasher Mode`**
- **Miele-Run-State → Matter `Operational State`**
- **Miele-Fehler → Matter `Dishwasher Alarm` + `Operational State = Error`**
- **Miele aktiv/inaktiv → Matter `On/Off` mit Dead-Front-Verhalten**
- **Miele Temperatur-/Stufenmodell → Matter `Temperature Control`**

## Quellen

### Matter 1.5.1
- Matter Device Library Specification, Dishwasher Device Type, Dishwasher Mode, Dishwasher Alarm
- Matter Application Cluster Specification, Operational State Cluster, Dishwasher Mode Cluster, Dishwasher Alarm Cluster
- Matter Standard Namespaces, semantic tags for `Normal`, `Heavy`, `Light`

### Miele 3rd Party API
- Get Started / General Concept
- Device Capabilities
- Authorization
- Eventing
- Best Practices
- Swagger / API Explorer

---

**Hinweis:** Dieses Dokument ist als technische Mapping-Grundlage für eine MatterBridge gedacht. Für produktiven Code sollten die konkreten Miele-JSON-Modelle und Aktionen noch gegen die aktuelle API-Definition verifiziert werden.

