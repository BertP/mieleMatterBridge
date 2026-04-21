# Rollenabgrenzung: `firmware_coder` vs. `expert_embedded_coder`

## Zweck

Dieses Dokument beschreibt die klare fachliche und technische Abgrenzung zwischen den Agenten `firmware_coder` und `expert_embedded_coder` im Projekt **Miele Matter Bridge PoC auf ESP32-S3-N8R2**.

## Grundprinzip

- **`firmware_coder`** baut die **Produktfunktion**.
- **`expert_embedded_coder`** sichert die **Embedded-Qualitaet der Laufzeitplattform**.

Kurzform:

- `firmware_coder` fragt: **Was soll das Geraet koennen?**
- `expert_embedded_coder` fragt: **Laeuft das auf dem ESP32-S3 stabil, sicher und dauerhaft?**

## Zustaendigkeit nach Themen

| Thema | firmware_coder | expert_embedded_coder |
|---|---|---|
| Matter-Cluster-Logik | fuehrend | beratend |
| API -> Matter-Mapping | fuehrend | beratend |
| Command-Handling | fuehrend | beratend |
| FreeRTOS-Architektur | mitwirkend | fuehrend |
| Speicherbudget / Heap / Stack | unterstuetzend | fuehrend |
| TLS / HTTP-Robustheit | mitwirkend | fuehrend |
| NVS / Persistenzstrategie | mitwirkend | fuehrend |
| Watchdog / Recovery | unterstuetzend | fuehrend |
| Logging / Tracing | gemeinsam | gemeinsam |
| Buildbare Feature-Inkremente | fuehrend | unterstuetzend |
| Stabilisierung vor Demo | unterstuetzend | fuehrend |

## Konkrete Trennung

### `firmware_coder` besitzt
- `dishwasher_mapper.*`
- `matter_endpoint_factory.*`
- `operational_state_adapter.*`
- `dishwasher_mode_adapter.*`
- Feature-seitige Sync-Orchestrierung
- Command-Pfade fuer Start, Stop, Pause
- Read-Model fuer Restlaufzeit, Programm und Status

### `expert_embedded_coder` besitzt
- `network_resilience.*`
- `http_client_tuning.*`
- `tls_time_sync.*`
- `nvs_store.*`
- `task_layout.*`
- `heap_monitor.*`
- `crash_recovery.*`
- Low-Level Buffer-, Retry- und Backoff-Strategien

## Entscheidungsregel bei Ueberschneidungen

- **Feature-Semantik gewinnt `firmware_coder`**
- **Laufzeitverhalten gewinnt `expert_embedded_coder`**

## Workflow

1. `matter_api_mapper` definiert das Mapping.
2. `firmware_coder` implementiert die fachliche Umsetzung.
3. `expert_embedded_coder` haertet den Stand fuer den MCU-Betrieb.
4. `validation_tester` prueft Funktion und Fehlerpfade.
5. `technical_writer` dokumentiert Verhalten, Limits und bekannte Randfaelle.
