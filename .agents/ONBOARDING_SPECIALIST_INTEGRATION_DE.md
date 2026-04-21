# Integration des Matter Onboarding-Spezialisten

Dieser Zusatzagent erweitert das bestehende Setup um eine gezielte Rolle fuer Onboarding-, Pairing- und Discovery-Probleme bei Matter auf dem ESP32-S3.

## Wann einsetzen
- QR-Code oder Manual Pairing Code werden akzeptiert, aber das Geraet erscheint nicht stabil im Oekosystem
- BLE-Rendezvous funktioniert nicht oder bricht frueh ab
- Wi-Fi-Join, mDNS oder Fabric-Zustand sind unklar
- `chip-tool` und Controller-App verhalten sich unterschiedlich
- Nach Factory Reset oder Neu-Flashen bleibt das Commissioning inkonsistent

## Empfohlene Position im Ablauf
1. `system_architect`
2. `matter_api_mapper`
3. `delivery_planner`
4. `firmware_coder`
5. `expert_embedded_coder`
6. `matter_commissioning_forensics`
7. `validation_tester`
8. `technical_writer`

## Typische Artefakte
- Onboarding Payload Checkliste
- Known-Good `chip-tool` Commissioning Flow
- Pairing-Fehlerklassifikation nach Phase
- Recovery-Runbook fuer Factory Reset und Recommissioning
