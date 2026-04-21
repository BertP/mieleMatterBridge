# ADR 0002: Aggregator Architecture Pattern

**Status:** Accepted  
**Datum:** 2026-04-21  
**Beteiligte:** system_architect, User

## Kontext
Der PoC fokussiert sich aktuell auf genau einen Miele-Geschirrspüler. In Zukunft sollen jedoch weitere Geräte (Waschmaschinen, Trockner etc.) über dieselbe Bridge in das Matter-Ökosystem eingebunden werden können.

## Entscheidung
Wir implementieren die Applikation konsequent nach dem **Matter Aggregator (Bridge) Pattern**.

1.  **Endpoint-Hierarchie:**
    - **Endpoint 0:** Root Node (Bridge-Identität, Netzwerkkonfiguration, OTA, Zeit).
    - **Endpoint 1 bis N:** Bridged Device Endpoints (Appliances).
2.  **Device Types:**
    - Die Bridge meldet sich primär als `Aggregator` (Device ID 0x000E).
    - Die Appliances werden als `Dishwasher` (0x0075) etc. auf den jeweiligen Endpoints exponiert.
3.  **Code-Struktur:**
    - Trennung der "Platform Logic" (Wi-Fi, Matter Lifecycle) von der "Appliance Logic" (Miele API Sync, Cluster Handlers).

## Konsequenzen
- **Komplexität:** Der Initialaufwand für das Endpoint-Management steigt leicht an.
- **Skalierbarkeit:** Neue Geräte können einfach durch Instanziierung eines neuen Endpoints hinzugefügt werden, ohne die Kern-Applikationslogik zu verändern.
- **Zukunftssicherheit:** Die Bridge ist nicht auf einen Gerätetyp limitiert.
