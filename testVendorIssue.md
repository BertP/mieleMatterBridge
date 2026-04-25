Exzellentes Dokument — du hast das Problem bereits sehr präzise beschrieben. Hier ist meine vollständige Analyse und Bewertung als Senior Developer, plus konkrete Abhilfe-Szenarien.

***

# 🔍 Senior Review: `TEST_VENDOR` Caching-Konflikt

## Bewertung der Ursachen-Analyse

Die im Dokument beschriebene Ursachen-Analyse ist **technisch korrekt und vollständig**. Beide Kernprobleme greifen tatsächlich ineinander — das macht diesen Bug so hartnäckig: [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/57009900/67c277bf-3f87-4859-8b59-51cf1c47549e/Test_Vendor01.md?AWSAccessKeyId=ASIA2F3EMEYEZAPA6FOZ&Signature=yzGyYLJ%2Fvo6RDqvqhuEbzc1UJTk%3D&x-amz-security-token=IQoJb3JpZ2luX2VjEMj%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FwEaCXVzLWVhc3QtMSJGMEQCIFEb7tFmnOtLBNeg%2FjKDr8EtR0WemCnrVr0Obn03fiFvAiBdfapI0Lcx%2FV9C8gOND4748so6IIZEEtw0P%2FRqhFiIMCr8BAiR%2F%2F%2F%2F%2F%2F%2F%2F%2F%2F8BEAEaDDY5OTc1MzMwOTcwNSIMqDpMXK5kycfeyytCKtAEGnDRTvDVg0Q3FLGQDJIrIwxcyYr0%2BX5HmakiNl94oJ4ha8W0bYJ3qoe6hoH2O8APEQaGqaFUjIpg9dUuA1UP%2FVgTo47o5gPpacE2MCg0Y3B6eH1%2Bwo%2BWFjEcaqBShpp7sKhenahGzQtTX1p7VFiteYRHrNVc56aECQz4dfksw4dkWazU4UBL7kxi1G0gHOcIIOUFOM%2BhUr1swYwVjed6zihLUebcTAFR2aahcIEIwKhAGnsW6fT9q8kLH1q%2F4Mmxm7UIQCDLIRVnM3eQyHntfqmoOm2iCFVAlOXFuEmpQlU3ypRPRH7H3ECbbpi450hwBG%2FhsiZWIiomGnsfWrQnIo3FEii6VDsVfelJx5Tq3kIQY0lfWOaHDhpkhbAZhjC7g8KVjuCOoIri9XMx%2BmLMPEkwxqVNkUQCcH5zP6q8MbEykzfb1Wb3RrugNLRtqxpVXx6G%2BDgsNcbgL%2F1IeWk4PbEfhxR%2BVoqHWAEt3nBqNnpNHqxCgVitCeNV0Z%2BRkOtzzEKwhhvhez1tr0B2PIzj1dOPsZEaAd1CIzgM6%2F2m8ctJdp9X7QgvDyvqa0wQU3oiStuUVHJNZspuTUPx7ELjrWaD7zAAAnFaC9eyGl3g2um7ykvEpDjiPVYKBUDgEPQKZq3DTEECRIdWUQAO7hWGmUYrSdmRWO0ie8Ski5j5h7xF8WUMObJ8O3l7AntTrzfYy8Buqdkmd3QqxmQUJhb9OmLdrzeQScBUSyj9j0YR2w7GvSJav6y87TVcbUpb55NgAMVyMT4PQAaGQmm%2FLsXGojDr5LHPBjqZAXufUVRLX%2BG3glJZc%2FawlQ1b5kJ2wWZhHwg3WpM%2B%2BmbSxjH2dujSKF3zvXh%2FmLHLNurzxPDGCgR0MFFAnXtozX6KdIgaUZ2DWUmVJJDJ8V4RagiK7xVReR3f0pKm%2Fm1J%2FFzQpJreOe4rYOoTn1ks2r9eCu7GCNV1Zn%2BYaRU8YRhXQ3nYLyxH1a8%2FnbIGuYP8X5MC%2F0YUkdCj3Q%3D%3D&Expires=1777104796)

| Problem | Bewertung | Schwere |
|---|---|---|
| Google Home Cloud-Caching beim ersten Pairing | ✅ Korrekt erkannt | 🔴 Hoch |
| Test-VID `0xFFF1` → hartes `TEST_VENDOR`-Mapping | ✅ Korrekt erkannt | 🔴 Fundamental |
| Software-Version zeigt noch `2.3.7` trotz `v2.3.9` | ✅ Folge des Cachings | 🟡 Mittel |

**Wichtige Ergänzung:** Die drei Code-Maßnahmen in Abschnitt 3 (Makros, `DeviceInstanceInfoProvider`, `attribute::update`) bekämpfen alle **das falsche Ende des Problems**. Das Gerät sendet die richtigen Werte — Google Home liest sie nur **nach dem ersten Pairing schlicht nicht mehr ab**. Du kannst im Gerät schreiben was du willst, solange Google Home den Cache nicht invalidiert. [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/57009900/67c277bf-3f87-4859-8b59-51cf1c47549e/Test_Vendor01.md?AWSAccessKeyId=ASIA2F3EMEYEZAPA6FOZ&Signature=yzGyYLJ%2Fvo6RDqvqhuEbzc1UJTk%3D&x-amz-security-token=IQoJb3JpZ2luX2VjEMj%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FwEaCXVzLWVhc3QtMSJGMEQCIFEb7tFmnOtLBNeg%2FjKDr8EtR0WemCnrVr0Obn03fiFvAiBdfapI0Lcx%2FV9C8gOND4748so6IIZEEtw0P%2FRqhFiIMCr8BAiR%2F%2F%2F%2F%2F%2F%2F%2F%2F%2F8BEAEaDDY5OTc1MzMwOTcwNSIMqDpMXK5kycfeyytCKtAEGnDRTvDVg0Q3FLGQDJIrIwxcyYr0%2BX5HmakiNl94oJ4ha8W0bYJ3qoe6hoH2O8APEQaGqaFUjIpg9dUuA1UP%2FVgTo47o5gPpacE2MCg0Y3B6eH1%2Bwo%2BWFjEcaqBShpp7sKhenahGzQtTX1p7VFiteYRHrNVc56aECQz4dfksw4dkWazU4UBL7kxi1G0gHOcIIOUFOM%2BhUr1swYwVjed6zihLUebcTAFR2aahcIEIwKhAGnsW6fT9q8kLH1q%2F4Mmxm7UIQCDLIRVnM3eQyHntfqmoOm2iCFVAlOXFuEmpQlU3ypRPRH7H3ECbbpi450hwBG%2FhsiZWIiomGnsfWrQnIo3FEii6VDsVfelJx5Tq3kIQY0lfWOaHDhpkhbAZhjC7g8KVjuCOoIri9XMx%2BmLMPEkwxqVNkUQCcH5zP6q8MbEykzfb1Wb3RrugNLRtqxpVXx6G%2BDgsNcbgL%2F1IeWk4PbEfhxR%2BVoqHWAEt3nBqNnpNHqxCgVitCeNV0Z%2BRkOtzzEKwhhvhez1tr0B2PIzj1dOPsZEaAd1CIzgM6%2F2m8ctJdp9X7QgvDyvqa0wQU3oiStuUVHJNZspuTUPx7ELjrWaD7zAAAnFaC9eyGl3g2um7ykvEpDjiPVYKBUDgEPQKZq3DTEECRIdWUQAO7hWGmUYrSdmRWO0ie8Ski5j5h7xF8WUMObJ8O3l7AntTrzfYy8Buqdkmd3QqxmQUJhb9OmLdrzeQScBUSyj9j0YR2w7GvSJav6y87TVcbUpb55NgAMVyMT4PQAaGQmm%2FLsXGojDr5LHPBjqZAXufUVRLX%2BG3glJZc%2FawlQ1b5kJ2wWZhHwg3WpM%2B%2BmbSxjH2dujSKF3zvXh%2FmLHLNurzxPDGCgR0MFFAnXtozX6KdIgaUZ2DWUmVJJDJ8V4RagiK7xVReR3f0pKm%2Fm1J%2FFzQpJreOe4rYOoTn1ks2r9eCu7GCNV1Zn%2BYaRU8YRhXQ3nYLyxH1a8%2FnbIGuYP8X5MC%2F0YUkdCj3Q%3D%3D&Expires=1777104796)

***

## Antworten auf die offenen Expertenfragen

### 1. Cache-Busting — Wie den Google-Home-Cache sauber invalidieren?

Das ist die **schwierigste** der drei Fragen. Google Home invalidiert den Fabric-Cache eines Geräts **nur** unter folgenden Bedingungen:

- Das Gerät verlässt die Fabric sauber (Matter `RemoveFabric`-Command) → Google Home löscht den Eintrag
- Das Gerät meldet eine neue **Node-ID** auf der gleichen Fabric → selten praktisch nutzbar
- **Der zuverlässigste Entwickler-Workaround:** Gerät vollständig aus dem Google Home entfernen (Gerät löschen, nicht nur resetten), dann frisch pairen

**Konkrete Schritte für den Entwickleralltag:**
```
1. Google Home App → Gerät auswählen → "Gerät entfernen"
2. Auf dem ESP32-S3: idf.py erase-flash
3. Neue Firmware flashen
4. Frisch pairen → Google Home liest beim ersten Commissioning alle Attribute neu ein
```

Zum Bluetooth/GATT-Cache-Problem unter Android: **Ja**, das ist real. Android cached GATT-Dienste aggressiv. Abhilfe:
```
Android Einstellungen → Apps → Bluetooth → Cache leeren
```

***

### 2. Branding trotz Test-VID `0xFFF1` — Ist das überhaupt möglich?

**Kurze Antwort: Nein, nicht in der Google Home Production-App.**

Die VID `0xFFF1` ist im Google-Ökosystem fest als Test-Vendor reserviert. Google Home mappt diese VID **intern hart** auf `TEST_VENDOR`/`TEST_PRODUCT`, unabhängig davon, was das Gerät im `Basic Information Cluster` meldet. Das ist eine bewusste Design-Entscheidung von Google, um Test-Geräte klar von Produktionsgeräten zu trennen. [ppl-ai-file-upload.s3.amazonaws](https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/57009900/67c277bf-3f87-4859-8b59-51cf1c47549e/Test_Vendor01.md?AWSAccessKeyId=ASIA2F3EMEYEZAPA6FOZ&Signature=yzGyYLJ%2Fvo6RDqvqhuEbzc1UJTk%3D&x-amz-security-token=IQoJb3JpZ2luX2VjEMj%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FwEaCXVzLWVhc3QtMSJGMEQCIFEb7tFmnOtLBNeg%2FjKDr8EtR0WemCnrVr0Obn03fiFvAiBdfapI0Lcx%2FV9C8gOND4748so6IIZEEtw0P%2FRqhFiIMCr8BAiR%2F%2F%2F%2F%2F%2F%2F%2F%2F%2F8BEAEaDDY5OTc1MzMwOTcwNSIMqDpMXK5kycfeyytCKtAEGnDRTvDVg0Q3FLGQDJIrIwxcyYr0%2BX5HmakiNl94oJ4ha8W0bYJ3qoe6hoH2O8APEQaGqaFUjIpg9dUuA1UP%2FVgTo47o5gPpacE2MCg0Y3B6eH1%2Bwo%2BWFjEcaqBShpp7sKhenahGzQtTX1p7VFiteYRHrNVc56aECQz4dfksw4dkWazU4UBL7kxi1G0gHOcIIOUFOM%2BhUr1swYwVjed6zihLUebcTAFR2aahcIEIwKhAGnsW6fT9q8kLH1q%2F4Mmxm7UIQCDLIRVnM3eQyHntfqmoOm2iCFVAlOXFuEmpQlU3ypRPRH7H3ECbbpi450hwBG%2FhsiZWIiomGnsfWrQnIo3FEii6VDsVfelJx5Tq3kIQY0lfWOaHDhpkhbAZhjC7g8KVjuCOoIri9XMx%2BmLMPEkwxqVNkUQCcH5zP6q8MbEykzfb1Wb3RrugNLRtqxpVXx6G%2BDgsNcbgL%2F1IeWk4PbEfhxR%2BVoqHWAEt3nBqNnpNHqxCgVitCeNV0Z%2BRkOtzzEKwhhvhez1tr0B2PIzj1dOPsZEaAd1CIzgM6%2F2m8ctJdp9X7QgvDyvqa0wQU3oiStuUVHJNZspuTUPx7ELjrWaD7zAAAnFaC9eyGl3g2um7ykvEpDjiPVYKBUDgEPQKZq3DTEECRIdWUQAO7hWGmUYrSdmRWO0ie8Ski5j5h7xF8WUMObJ8O3l7AntTrzfYy8Buqdkmd3QqxmQUJhb9OmLdrzeQScBUSyj9j0YR2w7GvSJav6y87TVcbUpb55NgAMVyMT4PQAaGQmm%2FLsXGojDr5LHPBjqZAXufUVRLX%2BG3glJZc%2FawlQ1b5kJ2wWZhHwg3WpM%2B%2BmbSxjH2dujSKF3zvXh%2FmLHLNurzxPDGCgR0MFFAnXtozX6KdIgaUZ2DWUmVJJDJ8V4RagiK7xVReR3f0pKm%2Fm1J%2FFzQpJreOe4rYOoTn1ks2r9eCu7GCNV1Zn%2BYaRU8YRhXQ3nYLyxH1a8%2FnbIGuYP8X5MC%2F0YUkdCj3Q%3D%3D&Expires=1777104796)

**Es gibt jedoch eine offizielle Alternative: Google Home Developer Console + CSA-DCL**

Für Entwickler/Prototyping gibt es die **Google Home Developer Console**, in der du deine eigene VID + PID registrieren und mit Custom-Branding versehen kannst — auch mit Test-Zertifikaten:

```
https://console.home.google.com
→ "Matter Integration" erstellen
→ VID: 0xFFF1, PID: deine PID eintragen
→ Branding (Name, Logo) hinterlegen
→ Tester-Email-Adressen freischalten
```

Danach respektiert Google Home das Branding aus der Developer Console **auch für Test-VIDs**, aber **nur für freigeschaltete Tester-Accounts**.

***

### 3. Selbstsignierte Zertifikate für VID `0x120E`

**Für Prototyping gibt es genau einen sauberen Weg: CHIP Tool + Custom PAA**

Das CHIP SDK erlaubt das Einbringen einer eigenen PAA (Product Attestation Authority) in eine Development-Build von Google Home. Für die **offizielle Google Home App** ist das nicht möglich — sie akzeptiert nur Zertifikate aus dem offiziellen CSA DCL (Distributed Compliance Ledger).

**Empfohlener Entwicklungspfad für Miele:**

```
Kurzfristig (jetzt, Prototyp):
  → Developer Console + VID 0xFFF1 + PID 0x8001
  → Custom Branding über Console hinterlegen
  → Tester-Accounts freischalten

Mittelfristig (Zertifizierung):
  → Miele VID 0x120E bei der CSA (Connectivity Standards Alliance) registrieren
  → PAA/PAI/DAC-Zertifikate von einem akkreditierten Aussteller beziehen
  → Mit Produktionszertifikaten neu pairen → kein TEST_VENDOR mehr

Langfristig:
  → Matter-Zertifizierung durchlaufen → Eintrag im öffentlichen DCL
  → Alle Ökosysteme (Google, Apple, Amazon) zeigen automatisch korrekte Metadaten
```

***

## Empfohlene Sofortmaßnahme

Die schnellste Lösung für euren Entwickleralltag ist Folgendes:

1. **Google Home Developer Console** aufsetzen, VID `0xFFF1` + PID `0x8001` mit Miele-Branding registrieren
2. Alle Entwickler-Accounts als Tester eintragen
3. Gerät **komplett** aus Google Home löschen (nicht nur resetten) und **frisch** pairen

Das gibt euch sauberes Branding in der Entwicklungsphase, ohne auf Produktionszertifikate warten zu müssen.