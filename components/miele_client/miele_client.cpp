#include "miele_client.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "cJSON.h"
#include "nvs_flash.h"
#include <sstream>
#include <iomanip>

static const char *TAG = "MIELE_CLIENT";

namespace miele {
namespace api {

static config_t s_config;

static std::string url_encode(const std::string &value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }
    return escaped.str();
}

static esp_err_t save_tokens_to_nvs() {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("miele_api", NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        nvs_set_str(handle, "access_token", s_config.access_token.c_str());
        nvs_set_str(handle, "refresh_token", s_config.refresh_token.c_str());
        nvs_commit(handle);
        nvs_close(handle);
        ESP_LOGI(TAG, "Tokens sicher im NVS gespeichert.");
    }
    return err;
}

static esp_err_t load_tokens_from_nvs() {
    nvs_handle_t handle;
    esp_err_t err = nvs_open("miele_api", NVS_READONLY, &handle);
    if (err == ESP_OK) {
        size_t required_size;
        
        // 1. Access Token
        if (nvs_get_str(handle, "access_token", NULL, &required_size) == ESP_OK) {
            char* buf = (char*)malloc(required_size);
            if (buf) {
                nvs_get_str(handle, "access_token", buf, &required_size);
                s_config.access_token = buf;
                free(buf);
            }
        }

        // 2. Refresh Token
        if (nvs_get_str(handle, "refresh_token", NULL, &required_size) == ESP_OK) {
            char* buf = (char*)malloc(required_size);
            if (buf) {
                nvs_get_str(handle, "refresh_token", buf, &required_size);
                s_config.refresh_token = buf;
                free(buf);
            }
        }
        
        nvs_close(handle);
        if (!s_config.access_token.empty()) {
            ESP_LOGI(TAG, "Tokens erfolgreich aus NVS geladen.");
            return ESP_OK;
        }
    }
    return ESP_FAIL;
}

esp_err_t init(const config_t &config) {
    s_config = config;
    ESP_LOGI(TAG, "Miele Client initialisierend...");
    
    // Versuche Tokens aus NVS zu laden
    if (load_tokens_from_nvs() != ESP_OK) {
        ESP_LOGW(TAG, "Keine Tokens im NVS gefunden. Onboarding erforderlich.");
    }
    
    return ESP_OK;
}

/**
 * @brief HTTP Event Handler zum Lesen des Response Bodies
 */
static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    std::string *response = (std::string *)evt->user_data;
    if (evt->event_id == HTTP_EVENT_ON_DATA) {
        if (response) {
            response->append((char *)evt->data, evt->data_len);
        }
    }
    return ESP_OK;
}

/**
 * @brief Basis-Funktion für Miele API Requests
 */
static esp_err_t perform_request(const char *url, esp_http_client_method_t method, const char *post_data, std::string &out_response) {
    out_response.clear();
    esp_http_client_config_t config = {};
    config.url = url;
    config.method = method;
    config.timeout_ms = 20000;
    config.buffer_size = 8192;
    config.buffer_size_tx = 8192;
    config.event_handler = _http_event_handler;
    config.user_data = &out_response;
    config.crt_bundle_attach = esp_crt_bundle_attach;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) return ESP_FAIL;

    esp_http_client_set_header(client, "Accept", "application/json");
    if (!s_config.access_token.empty()) {
        std::string auth_header = "Bearer " + s_config.access_token;
        esp_http_client_set_header(client, "Authorization", auth_header.c_str());
    }

    if (post_data) {
        esp_http_client_set_post_field(client, post_data, strlen(post_data));
        esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code >= 200 && status_code < 300) {
            ESP_LOGI(TAG, "Miele API Erfolg: %d", status_code);
        } else {
            ESP_LOGE(TAG, "Miele API Fehler: %d | Response: %s", status_code, out_response.c_str());
            err = ESP_FAIL;
        }
    } else {
        ESP_LOGE(TAG, "Netzwerkfehler: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return err;
}

bool is_authenticated() {
    return !s_config.access_token.empty();
}

esp_err_t get_devices(std::string &response) {
    if (s_config.access_token.empty()) {
        ESP_LOGE(TAG, "Kein Access Token vorhanden!");
        return ESP_ERR_INVALID_STATE;
    }

    std::string url = "https://api.mcs3.miele.com/v1/devices";
    return perform_request(url.c_str(), HTTP_METHOD_GET, NULL, response);
}

esp_err_t exchange_code(const std::string &code, const std::string &redirect_uri) {
    ESP_LOGI(TAG, "Tausche Authorization Code gegen Tokens...");
    const char *token_url = "https://auth.domestic.miele-iot.com/partner/realms/mcs/protocol/openid-connect/token";

    std::string body = "grant_type=authorization_code";
    body += "&client_id=" + s_config.client_id;
    body += "&client_secret=" + s_config.client_secret;
    body += "&code=" + url_encode(code);
    body += "&redirect_uri=" + url_encode(redirect_uri);

    std::string response;
    esp_err_t err = perform_request(token_url, HTTP_METHOD_POST, body.c_str(), response);

    if (err == ESP_OK) {
        cJSON *root = cJSON_Parse(response.c_str());
        if (root) {
            cJSON *at = cJSON_GetObjectItem(root, "access_token");
            cJSON *rt = cJSON_GetObjectItem(root, "refresh_token");
            if (at && rt) {
                s_config.access_token = at->valuestring;
                s_config.refresh_token = rt->valuestring;
                ESP_LOGI(TAG, "Tokens erfolgreich erhalten!");
                save_tokens_to_nvs();
            }
            cJSON_Delete(root);
        }
    }
    return err;
}

esp_err_t refresh_token() {
    ESP_LOGI(TAG, "Erneuere Access Token...");
    const char *token_url = "https://auth.domestic.miele-iot.com/partner/realms/mcs/protocol/openid-connect/token";
    
    std::string body = "grant_type=refresh_token";
    body += "&client_id=" + s_config.client_id;
    body += "&client_secret=" + s_config.client_secret;
    body += "&refresh_token=" + s_config.refresh_token;

    std::string response;
    esp_err_t err = perform_request(token_url, HTTP_METHOD_POST, body.c_str(), response);

    if (err == ESP_OK) {
        cJSON *root = cJSON_Parse(response.c_str());
        if (root) {
            cJSON *at = cJSON_GetObjectItem(root, "access_token");
            if (at) {
                s_config.access_token = at->valuestring;
                ESP_LOGI(TAG, "Access Token erneuert.");
            }
            cJSON_Delete(root);
        }
    }
    return err;
}

esp_err_t get_appliance_status(const std::string &device_id, appliance_status_t *out_status) {
    if (!out_status) return ESP_ERR_INVALID_ARG;
    std::string url = "https://api.mcs3.miele.com/v1/devices/" + device_id;
    std::string response;
    return perform_request(url.c_str(), HTTP_METHOD_GET, NULL, response);
}

} // namespace api
} // namespace miele
