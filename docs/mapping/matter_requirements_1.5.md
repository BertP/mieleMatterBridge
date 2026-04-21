# Matter 1.5.1 Anforderungen: Dishwasher (Device ID: 0x0075)

Basierend auf der offiziellen Matter 1.5.1 Spezifikation (Device Library & Application Cluster).

## 1. Obligatorische Cluster (Application Layer)

### Operational State Cluster (0x0060) - [M]
Der zentrale Cluster zur Steuerung und Überwachung des Gerätezyklus.

**Wichtige Attribute:**
- `OperationalState`: Aktueller Zustand (0: Stopped, 1: Running, 2: Paused, 3: Error).
- `OperationalError`: Detail-Information bei Fehlern.
- `PhaseList`: Liste der Programmphasen (z.B. "Pre-wash", "Main wash", "Drying").
- `CurrentPhase`: Index der aktuellen Phase.
- `CountdownTime`: Restlaufzeit (geschätzt) in Sekunden.

**Wichtige Commands:**
- `Pause`
- `Resume`
- `Start`
- `Stop`

---

## 2. Empfohlene Cluster für den PoC

### Dishwasher Mode Cluster (0x0059) - [O]
Wird zur Auswahl des Reinigungsprogramms verwendet.

- Mappt "Miele Programs" (z.B. QuickPowerWash, Eco) auf Matter "Modes".
- Unterstützt Tags zur Standardisierung (z.B. `CommonCodes.Mode.Normal`).

### Dishwasher Alarm Cluster (0x005D) - [O]
Signalisierung von kritischen Zuständen.

- Mappt Miele Fehler (z.B. F11 - Ablaufleitung verstopft) auf vordefinierte Alarme.

---

## 3. Strategische Mapping-Themen (Miele <-> Matter)

| Miele Status / Feld | Matter Ziel-Attribut | Anmerkung |
|---|---|---|
| Program Status (z.B. "In use") | `OperationalState` | Miele "In use" -> Matter "Running" |
| Remaining Time | `CountdownTime` | Einheitensequenz prüfen (Miele: Min -> Matter: Sek) |
| Program ID | `DishwasherMode` | Statische Liste der unterstützten Programme |
| Error Code | `OperationalError` | Enum-Mapping erforderlich |

---
**Dokumenten-Status:** Entwurf für Mapping-Baseline (2026-04-21)
