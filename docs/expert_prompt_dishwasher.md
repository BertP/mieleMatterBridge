# 🤖 Experten-Prompt: Cloud-to-Matter Mapping für Miele Spülmaschinen

*Kopiere diesen Text und füge ihn in Perplexity oder Claude Sonnet ein, um die optimale Datenmodell-Übersetzung zu erhalten.*

---

Hallo! Ich baue eine **Matter-Bridge** für Miele-Haushaltsgeräte und benötige Unterstützung beim logischen Datenmapping zwischen der **Miele Cloud API** und dem **Matter 1.1+ Datenmodell**.

**Wichtige Architektur-Info:** Der ESP32 dient als Proxy-Bridge für **mehrere** Miele-Endgeräte gleichzeitig (jedes Gerät bekommt einen eigenen Endpoint). Im ersten Schritt beginnen wir mit der **Spülmaschine auf Endpoint 1**.

### 1. Unser aktueller Datenstand (Miele API)
Wir rufen den Zustand der Spülmaschine über die Miele-Schnittstelle ab und erhalten in C++ folgendes Struct:

```cpp
struct appliance_status_t {
    std::string device_id;       // Seriennummer
    int status_id;               // Miele Gerätezustand (z.B. 5=In Betrieb, 7=Fertig)
    int program_id;              // Ausgewähltes Programm
    int remaining_time_mins;     // Restlaufzeit in Minuten
    bool is_on;                  // Power State
};
```

Zusätzlich unterstützt die Miele API Server-Sent Events (SSE) für Echtzeit-Events.

### 2. Die Frage für das Matter-Mapping:
Ich möchte diese Daten auf einen **Matter Endpoint 1 (Dishwasher)** abbilden.

* **Operational State Cluster:** Wie übersetze ich die `status_id` (Miele Enum) am besten in die Matter-Zustände (`Phase`, `OperationalState`, `ErrorState`)?
* **Dishwasher Mode Cluster:** Wie mappt man die `program_id` am besten auf Matter Modes?
* **Time Remaining:** Welcher Matter-Cluster verarbeitet die Restlaufzeit (`remaining_time_mins`)?

Gib mir bitte eine logische Übersetzungstabelle (Miele-Zustand ➡️ Matter-Attribut) sowie Ratschläge, wie man unvollständige Daten aus der Cloud sinnvoll im Matter-Cluster puffert.

