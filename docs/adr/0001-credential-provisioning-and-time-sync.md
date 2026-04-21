# ADR-0001: Credential Provisioning & Time Sync Strategy

**Status:** Proposed  
**Owner:** `system_architect`  
**Reviewers:** `expert_embedded_coder`, `firmware_coder`  
**Date:** 2026-04-21

## Context

The Miele 3rd Party API requires OAuth2 (Client ID, Client Secret, Access Token, Refresh Token) and HTTPS (TLS). 
Matter requires synchronized time for protocol operations.
The ESP32-S3 needs:
1. Valid system time (NTP) to verify TLS certificates and Matter timestamps.
2. Root CA certificates to verify the Miele API server.
3. OAuth2 credentials to authenticate requests.
4. User tokens to access dishwasher data.

The project mission specifies "No Hardcoded Credentials" and "No Mobile App" for the PoC.

## Decision

We will implement the following strategies for the PoC:

### 1. Time Synchronization
- Use **NTP (Network Time Protocol)**.
- Default to `pool.ntp.org`.
- Implementation will block the Miele API Client initialization until a valid time sync is achieved (SNTP sync status).

### 2. Root CA Certificates
- Use a **PEM-formatted Root Certificate Bundle**.
- For the PoC, the certificate will be embedded in the firmware image as a resource (via `target_add_binary_data` or `component.mk`), but referenced via a pointer to ensure modularity. 
- *Rationale:* Easiest to maintain during PoC phase while keeping the code clean.

### 3. Credential Provisioning (Client ID / Secret)
- For the PoC, these will be injected via **Kconfig** and stored in **NVS** on first boot.
- They will be configurable via `idf.py menuconfig` for the developer.

### 4. User Token Management (Initial Authentication)
- Since no mobile app exists, we will use a **Semi-Manual NVS Injection**.
- The developer will provide the tokens (obtained once via a manual browser flow) through a specific CSV/NVS-Partition file or a simple serial command.
- **Decision:** Use an **NVS Partition CSV file** for the PoC to pre-fill the tokens during flashing.

### 5. Token Persistence & Refresh
- The `firmware_coder` must implement an automatic **Refresh Token Flow**.
- New tokens must be stored back to **NVS** immediately to survive reboots.
- The `expert_embedded_coder` must ensure the NVS namespace is wear-leveled and resilient.

## Consequences

- **Pros:** 
    - No sensitive data in Git.
    - Robustness against reboots.
    - Standard ESP-IDF tools can be used for provisioning.
- **Cons:** 
    - Initial setup requires developer access to `idf.py` or `nvs_flash_tool`.
    - No "Zero-Touch" onboarding for end-users (out of scope for PoC).

## Open Questions
- Should we provide a simple CLI via UART to update tokens at runtime without re-flashing? (Review required by `firmware_coder`).
