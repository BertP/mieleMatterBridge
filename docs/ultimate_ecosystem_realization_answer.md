<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" style="height:64px;margin-right:32px"/>

# Wenn du willst, kann ich dir als Nächstes eine konkrete Reset-/Re-Pair-/Re-Register-Checkliste schreiben, damit ihr sauber zwischen Firmware-Problem, Google-Cache und Developer-Console-Fehler unterscheiden könnt.

Das würde uns sehr helfen

Ja — hier ist eine **saubere Reset-/Re-Pair-/Re-Register-Checkliste**, mit der ihr systematisch trennt zwischen Firmware, Google-Cache und Developer-Console.[^1][^2][^3]

## Ziel der Checkliste

Die Idee ist, jeden Testdurchlauf so aufzubauen, dass nur **eine Variable** verändert wird. Dann seht ihr sofort, ob ein Problem aus dem ESP32-Image, aus Google Home oder aus der Developer Console kommt.[^3][^1]
Wichtig ist: Google empfiehlt zum Entfernen von Matter-Geräten ausdrücklich die Entfernen-Funktion in der Google Home App; danach soll das Gerät vor dem erneuten Hinzufügen zurückgesetzt werden.[^2][^1]

## Checkliste

### 1) Firmware isolieren

- Flash das Gerät neu mit einer eindeutig neuen Versionsnummer und eindeutigen Strings.
- Prüfe im Serienlog, dass **Advertising** und **Cluster-Attribute** dieselben Werte zeigen.
- Erst wenn im Log die neue PID/Version korrekt auftaucht, ist die Firmware-Seite sauber.[^4][^5]


### 2) Google Home löschen

- In der Google Home App das Gerät **vollständig entfernen**.
- Nicht nur „Reset“, sondern wirklich **Remove device** in der App verwenden.[^1][^2]
- Danach das Gerät am Controller/Telefon ebenfalls aus allen lokalen Matter-Listen entfernen, falls vorhanden.[^6][^1]


### 3) Gerät hart zurücksetzen

- ESP32 vollständig neu flashen oder erase-flash ausführen.
- Danach prüfen, dass das Gerät wieder im Pairing-/Commissioning-Modus ist.
- So stellt ihr sicher, dass kein lokaler Zustand vom alten Pairing übrig bleibt.[^7][^6]


### 4) Developer Console prüfen

- Im Google Home Developer Console Projekt die Matter-Integration kontrollieren.
- Alte Testgeräte, alte Versionen oder alte PID/VID-Kombinationen entfernen bzw. ein neues Testprojekt anlegen, wenn nötig.[^8][^3]
- Bei laufenden Integrationen kann Google selbst Löschungen oder Änderungen einschränken; ein neues Testprojekt ist oft schneller.[^8][^3]


### 5) Neu pairen

- Gerät mit **neuen** eindeutigen Werten erneut pairen.
- Erst jetzt Google Home wieder öffnen und die technischen Informationen prüfen.
- Wenn der Fehler wieder derselbe ist, ist die Ursache fast sicher ein Google-/Console-Cache und nicht die Firmware.[^3][^1]


## Entscheidungslogik

| Beobachtung | Wahrscheinlichste Ursache | Nächster Schritt |
| :-- | :-- | :-- |
| Logs zeigen alte PID/Version | Firmware/Kconfig nicht sauber neu gebaut | `sdkconfig` löschen, neu builden [^9] |
| Logs zeigen neue Werte, Google Home zeigt alte Werte | Google-Cache oder Console-Eintrag | Gerät löschen, Console bereinigen, neu pairen [^1][^3] |
| Alles zeigt alte Werte | Mehrere Cache-Schichten gleichzeitig | Komplettreset aller drei Ebenen [^1][^6][^3] |

## Die beste Testreihenfolge

1. `erase-flash` und Neu-Flash.
2. Serielle Logs prüfen.
3. Gerät aus Google Home entfernen.
4. Developer-Console-Testeintrag prüfen oder neu anlegen.
5. Neu pairen.
6. Ergebnis dokumentieren.

Damit könnt ihr sauber beweisen, **wo** der Zustand festhängt: Firmware, lokaler Matter-Cache oder Google-Cloud-/Developer-Console-Seite.[^1][^3]

## Praktischer Rat

Für euren Fall würde ich nach jedem Lauf eine kleine Matrix führen:

- **Build-ID**
- **Advertised PID**
- **Basic-Info-Version**
- **Google-Home-UI-Version**
- **Developer-Console-Projekt**
- **Neues oder altes Pairing**

So erkennt ihr sofort, ob Google wirklich nur einen alten Testeintrag wiederverwendet oder ob die neue Firmware noch nicht sauber im Build angekommen ist.[^5][^9]

Wenn du willst, schreibe ich dir daraus als Nächstes ein **kompaktes 1-Seiten-Testprotokoll zum Abhaken**.
<span style="display:none">[^10][^11][^12][^13][^14][^15][^16][^17][^18]</span>

<div align="center">⁂</div>

[^1]: https://support.google.com/googlenest/answer/13127223?hl=en-CA

[^2]: https://support.google.com/googlenest/answer/13127223?hl=en

[^3]: https://developers.home.google.com/matter/integration/delete

[^4]: Technischer-Abschlussbericht-D.txt

[^5]: ultimate_ecosystem_realization.md

[^6]: https://www.1home.io/docs/en/server/matter-controller/troubleshooting

[^7]: https://www.trunetto.com/troubleshooting/smart-hubs/nest/matter-device-not-working-google-home

[^8]: https://developers.home.google.com/matter/launch/delete

[^9]: final_expert_summary.md

[^10]: https://support.google.com/googlenest/answer/9293132?hl=en

[^11]: https://community.home-assistant.io/t/how-to-delete-matter-devices/611331

[^12]: https://www.home-assistant.io/integrations/matter/

[^13]: https://www.reddit.com/r/MatterProtocol/comments/1h28ad7/support_needed_cannot_readd_a_matter_device_to/

[^14]: https://www.reddit.com/r/googlehome/comments/1hfwb2w/how_can_i_remove_all_800_matter_devices_from_my/

[^15]: https://www.youtube.com/watch?v=w55pPggsRIM

[^16]: https://dev.to/antigones/matter-protocol-on-a-budget-32ph

[^17]: https://community.home-assistant.io/t/cant-remove-old-test-app-from-google-home/75573

[^18]: https://matterbridge.io/README.html

