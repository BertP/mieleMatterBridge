# Miele API OAuth2: Device-Specific Permissions

This document summarizes the technical requirements and parameters for integrating with the Miele 3rd Party API using the **OAuth2 Authorization Code Grant Flow with device-specific permissions**. This modern flow allows users to select individual appliances for access during the consent step.

## 1. API Endpoints

Always use the new domestic IoT endpoints for device-specific permissions. Legacy endpoints should be avoided for new developments.

| Type              | Endpoint URL                                                                           |
| :---------------- | :------------------------------------------------------------------------------------- |
| **Authorization** | `https://auth.domestic.miele-iot.com/partner/realms/mcs/protocol/openid-connect/auth`  |
| **Token**         | `https://auth.domestic.miele-iot.com/partner/realms/mcs/protocol/openid-connect/token` |

## 2. Authorization Scopes

Scopes define the level of access your application is requesting. Multiple scopes must be separated by a URL-encoded space (`%20`).

- **`openid`** (Mandatory): Required for technical protocol reasons.
- **`mcs_thirdparty_read`**: Grants read access to appliance states, details, and programs.
- **`mcs_thirdparty_write`**: Allows triggering actions (e.g., Power On, Start Program).
- **`mcs_thirdparty_media`**: Enables access to appliance-generated media (e.g., oven camera images).

## 3. Implementation Details

### A. Authorization Request Parameters

When redirecting the user to the Miele login page, the following parameters are required:

| Parameter       | Description                                                                 |
| :-------------- | :-------------------------------------------------------------------------- |
| `client_id`     | Your unique Client ID assigned during registration.                         |
| `response_type` | Must be `code`.                                                             |
| `redirect_uri`  | The URL Miele redirects to after authorization (must match registered URI). |
| `scope`         | The requested scopes (e.g., `openid mcs_thirdparty_read`).                  |
| `state`         | A random string to prevent CSRF attacks.                                    |

### B. Token Request Parameters

After receiving the authorization code, exchange it for an access token via a `POST` request to the Token URL:

| Parameter       | Value / Description                                             |
| :-------------- | :-------------------------------------------------------------- |
| `grant_type`    | `authorization_code`                                            |
| `client_id`     | Your Client ID.                                                 |
| `client_secret` | Your Client Secret.                                             |
| `code`          | The authorization code received from the redirect.              |
| `redirect_uri`  | Must be identical to the one used in the authorization request. |

### C. Token Response (JWT)

The server returns a JSON response containing:

- **`access_token`**: A signed JWT containing claims about the user's country and the specific **serial numbers** of the appliances granted.
- **`refresh_token`**: Used to obtain a new access token when the current one expires.
- **`expires_in`**: Lifetime of the access token in seconds.

## 4. Key Features of Device-Specific Flow

- **Granular Consent**: The user is presented with a list of their connected appliances and can manually select which ones the 3rd party application is allowed to access.
- **JWT Claims**: The access token itself contains the list of permitted serial numbers, allowing the client to verify access rights directly from the token.
