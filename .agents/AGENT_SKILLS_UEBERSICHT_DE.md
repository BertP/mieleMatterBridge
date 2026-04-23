# Agent Skills fuer MatterBridge ESP32-S3 PoC

Dieser Ordner beschreibt die Agentenrollen fuer die Entwicklung einer MatterBridge fuer einen Miele-Geschirrspueler auf ESP32-S3-N8R2.

## Enthaltene Rollen

- `system_architect` - System Architect: Definiert die Zielarchitektur des MatterBridge-PoC auf dem ESP32-S3-N8R2 und hält den Scope technisch sauber.
- `delivery_planner` - Delivery Planner: Zerlegt Architektur und Mapping in kleine, testbare und geordnete Arbeitspakete fuer ein agentisches Umsetzungssystem.
- `matter_api_mapper` - Matter/API Mapper: Uebersetzt die Miele-3rd-Party-API in ein sauberes Matter-Modell fuer den Dishwasher-PoC.
- `firmware_coder` - Firmware Coder: Implementiert die fachliche Firmware-Funktion des MatterBridge-PoC mit ESP-IDF und ESP-Matter.
- `expert_embedded_coder` - Expert Embedded Coder: Haertet die Firmware fuer den echten MCU-Betrieb auf dem ESP32-S3-N8R2.
- `validation_tester` - Validation Tester: Prueft Funktion, Robustheit und Scope-Treue des PoC mit reproduzierbaren Tests.
- `build_tooling_steward` - Build & Tooling Steward: Sichert reproduzierbare Builds, Toolchain-Konsistenz, CI und einheitliche Artefakte.
- `technical_writer` - Technischer Redakteur: Haelt Projektdokumente fachlich aktuell, strukturell konsistent und professionell lesbar.
- `ui_ux_designer` - UI/UX Designer: Entwirft die Bedienlogik und Oberflaechen fuer 128x64-Monochrom-Displays sowie fuer eine responsive Webapp auf Desktop und Smartphone.

## Weitere Dateien

- `handover_templates.yaml` - standardisierte Uebergabevorlagen zwischen den Agenten
- `ROLE_BOUNDARY_FIRMWARE_VS_EMBEDDED_DE.md` - klare Abgrenzung zwischen `firmware_coder` und `expert_embedded_coder`
- `role_boundary_firmware_vs_embedded.yaml` - maschinenlesbare Rollenabgrenzung
- `UI_UX_DESIGNER_INTEGRATION_DE.md` - Einordnung des UI/UX Designers im Setup
- `agent_skills_catalog.json` - Gesamtuebersicht aller Rollen und Dateien

## Empfohlene Reihenfolge

1. `system_architect`
2. `ui_ux_designer`
3. `matter_api_mapper`
4. `delivery_planner`
5. `firmware_coder`
6. `expert_embedded_coder`
7. `validation_tester`
8. `technical_writer`

Der `build_tooling_steward` arbeitet querschnittlich ueber alle Phasen.
