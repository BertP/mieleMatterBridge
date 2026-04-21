#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_matter.h"
#include "esp_matter_console.h"

// Headers for specific devices
#include <dish_washer_device.h>
#include <aggregator_device.h>
#include <app/server/Server.h>

// Miele API Client
#include "miele_client.h"
#include "esp_http_server.h"
#include "mdns.h"
#include "cJSON.h"

#include "ssd1306.h"
#include "driver/i2c.h"
#include "miele_matter_mapping.h"
#include "esp_wifi.h"

static const char *TAG = "MIELE_BRIDGE";
static ssd1306_t oled_dev;
static uint16_t dishwasher_endpoint_id = 0;

// Marker for embedded assets
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t miele_svg_start[]  asm("_binary_miele_svg_start");
extern const uint8_t miele_svg_end[]    asm("_binary_miele_svg_end");
extern const uint8_t matter_svg_start[] asm("_binary_matter_svg_start");
extern const uint8_t matter_svg_end[]   asm("_binary_matter_svg_end");

using namespace esp_matter;
using namespace esp_matter::endpoint;

// Credentials (aus docs/miele3rdpartyapi/credentials.txt)
#define MIELE_CLIENT_ID "0df585cb-2ac3-4bb0-8dd7-5fafe9e392fe"
#define MIELE_CLIENT_SECRET "60reBsf93NOCiPQiRS1Qxh04skNpoely"

// QR-Code Matrix (25x25) - 1 = Schwarz, 0 = Weiß
const int QR_SIZE = 25;
const uint8_t qr_matrix[25][25] = {
    {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0},
    {0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1},
    {0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0},
    {1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1},
    {0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0},
    {0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {1, 0, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0},
    {1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1},
};

static bool is_commissioned = false;

static void draw_qr_code() {
    uint8_t x_offset = 39; // Zentriert: (128 - 50) / 2
    
    // QR Code zeichnen (2x2 Skalierung)
    // Wir nutzen Page 1 bis 7 (Page 0 ist für den Titel)
    for (int p = 1; p <= 7; p++) {
        ssd1306_send_cmd(&oled_dev, 0xB0 + p);
        ssd1306_send_cmd(&oled_dev, x_offset & 0x0F);
        ssd1306_send_cmd(&oled_dev, 0x10 | (x_offset >> 4));
        
        uint8_t page_data[50];
        for (int module_x = 0; module_x < 25; module_x++) {
            uint8_t byte = 0;
            int r_start = (p - 1) * 4;
            for (int i = 0; i < 4; i++) {
                int r = r_start + i;
                if (r < 25 && qr_matrix[r][module_x]) {
                    byte |= (0x03 << (i * 2));
                }
            }
            page_data[module_x * 2] = byte;
            page_data[module_x * 2 + 1] = byte;
        }
        ssd1306_send_data(&oled_dev, page_data, 50);
    }
}

/**
 * @brief Geballte Matter-Events (Pairing-Status etc.)
 */
static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg) {
    switch (event->Type) {
        case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged: {
            ESP_LOGI(TAG, "IP-Adresse erhalten oder geändert.");
            // Wir versuchen die IP aufs OLED zu bringen, damit du siehst dass er im WLAN ist!
            ssd1306_draw_string(&oled_dev, 0, 7, "IP: CONNECTING.. ");
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
            ESP_LOGI(TAG, "Pairing erfolgreich abgeschlossen! 🎉");
            is_commissioned = true;
            ssd1306_clear(&oled_dev);
            ssd1306_draw_string(&oled_dev, 0, 0, "MIELE BRIDGE   ");
            ssd1306_draw_string(&oled_dev, 0, 7, "MATTER: OK      ");
            break;
        case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
            ESP_LOGW(TAG, "Fail-safe Timer abgelaufen!");
            break;
        default:
            break;
    }
}

/**
 * @brief GET / Handler: Serviert die Onboarding Seite
 */
static esp_err_t index_get_handler(httpd_req_t *req) {
    size_t size = index_html_end - index_html_start;
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, size);
    return ESP_OK;
}

/**
 * @brief GET /login Handler: Startet den Miele OAuth Flow
 */
static esp_err_t login_get_handler(httpd_req_t *req) {
    // Hier bauen wir die Miele Auth URL zusammen
    char redirect_url[512];
    snprintf(redirect_url, sizeof(redirect_url), 
        "https://auth.domestic.miele-iot.com/partner/realms/mcs/protocol/openid-connect/auth?"
        "client_id=%s&response_type=code&scope=openid%%20mcs_thirdparty_read%%20mcs_thirdparty_write&redirect_uri=http://miele-bridge.local/callback",
        MIELE_CLIENT_ID);
    
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", redirect_url);
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/**
 * @brief Asset Handler: Serviert das Miele Logo
 */
static esp_err_t miele_svg_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "image/svg+xml");
    httpd_resp_send(req, (const char *)miele_svg_start, miele_svg_end - miele_svg_start);
    return ESP_OK;
}

/**
 * @brief Asset Handler: Serviert das Matter Logo
 */
static esp_err_t matter_svg_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "image/svg+xml");
    httpd_resp_send(req, (const char *)matter_svg_start, matter_svg_end - matter_svg_start);
    return ESP_OK;
}

/**
 * @brief GET /callback Handler: Fängt den Authorization Code ab und zeigt edles Feedback
 */
static esp_err_t callback_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Callback-Aufruf empfangen: %s", req->uri);
    
    char *buf;
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    char code[256] = {0};

    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            httpd_query_key_value(buf, "code", code, sizeof(code));
        }
        free(buf);
    }

    if (strlen(code) > 0) {
        if (miele::api::exchange_code(code) == ESP_OK) {
            const char *success_html = 
                "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<style>body{background:#121212;color:white;font-family:sans-serif;display:flex;justify-content:center;align-items:center;height:100vh;margin:0; text-align:center;}"
                ".card{background:#1e1e1e;padding:40px;border-radius:20px;box-shadow:0 10px 30px rgba(0,0,0,0.5);width:80%;max-width:400px; border:1px solid #333;}"
                ".logos{display:flex;justify-content:center;gap:20px;margin-bottom:30px;align-items:center;}"
                ".logos img{height:40px;}"
                ".logos .sep{width:1px; height:30px; background:#444;}"
                "h1{color:#bc001a;margin:0 0 10px 0;font-size:24px;}"
                "p{color:#aaa;line-height:1.6;}"
                ".btn{display:inline-block;margin-top:20px;padding:12px 24px;background:#bc001a;color:white;text-decoration:none;border-radius:30px;font-weight:bold;}"
                "</style></head><body><div class='card'>"
                "<div class='logos'><img src='/assets/miele.svg'><div class='sep'></div><img src='/assets/matter.svg'></div>"
                "<h1>Verbunden!</h1>"
                "<p>Tokens erfolgreich generiert und sicher im NVS gespeichert.</p>"
                "<p>Die Bridge synchronisiert nun Ihre Ger&auml;te.</p>"
                "<a href='/' class='btn'>Zurück zur Übersicht</a>"
                "</div></body></html>";
            httpd_resp_set_type(req, "text/html; charset=utf-8");
            httpd_resp_send(req, success_html, strlen(success_html));
            return ESP_OK;
        }
    }

    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Fehler beim Code-Austausch");
    return ESP_OK;
}

/**
 * @brief 404 Handler zum Debuggen von falschen URIs
 */
static esp_err_t not_found_handler(httpd_req_t *req, httpd_err_code_t error) {
    ESP_LOGW(TAG, "Nicht gefundene URI angefragt: %s", req->uri);
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "URI nicht gefunden");
    return ESP_OK;
}

/**
 * @brief Hintergrund-Task für den Miele Cloud-Sync
 */
static void miele_sync_task(void *pvParameters) {
    ESP_LOGI(TAG, "Miele Sync Task gestartet.");
    
    while (1) {
        if (!is_commissioned) {
            draw_qr_code();
            vTaskDelay(pdMS_TO_TICKS(10000));
            continue;
        }

        if (miele::api::is_authenticated()) {
            ssd1306_draw_string(&oled_dev, 0, 2, "CLOUD: OK   ");
            ESP_LOGI(TAG, "Sync: Frage Miele Ger&auml;teliste ab...");
            std::string response;
            if (miele::api::get_devices(response) == ESP_OK) {
                ESP_LOGI(TAG, "Miele Cloud Discovery: %s", response.c_str());
                
                // Antwort parsen und gezielt den Geschirrspüler suchen
                cJSON* root = cJSON_Parse(response.c_str());
                if (root) {
                    cJSON* device = NULL;
                    cJSON* target_dishwasher = NULL;
                    
                    // Iteriere durch alle Fabriknummern (Keys)
                    cJSON_ArrayForEach(device, root) {
                        cJSON* ident = cJSON_GetObjectItem(device, "ident");
                        if (ident) {
                            cJSON* type = cJSON_GetObjectItem(ident, "type");
                            if (type && cJSON_GetObjectItem(type, "value_raw")->valueint == 7) {
                                target_dishwasher = device;
                                break; 
                            }
                        }
                    }

                    if (target_dishwasher) {
                        miele::mapping::update_dishwasher_attributes(dishwasher_endpoint_id, target_dishwasher);
                        
                        // OLED Live-Update
                        cJSON* state = cJSON_GetObjectItem(target_dishwasher, "state");
                        if (state) {
                            cJSON* status = cJSON_GetObjectItem(state, "status");
                            cJSON* rem = cJSON_GetObjectItem(state, "remainingTime");
                            if (status && rem) {
                                char buf[17];
                                // Status mit Padding zum Löschen alter Reste
                                snprintf(buf, sizeof(buf), "STA: %-11s", cJSON_GetObjectItem(status, "value_localized")->valuestring);
                                ssd1306_draw_string(&oled_dev, 0, 4, buf);
                                
                                snprintf(buf, sizeof(buf), "REM: %02d:%02d      ", 
                                        cJSON_GetArrayItem(rem, 0)->valueint,
                                        cJSON_GetArrayItem(rem, 1)->valueint);
                                ssd1306_draw_string(&oled_dev, 0, 5, buf);
                                
                                // Pairing Info am unteren Rand (FIXED PIN)
                                ssd1306_draw_string(&oled_dev, 0, 7, "PIN: 12345678   ");
                            }
                        }
                    } else {
                        ESP_LOGW(TAG, "Kein Geschirrspüler in der Cloud-Liste gefunden!");
                        ssd1306_draw_string(&oled_dev, 0, 4, "G7310 NOT FOUND");
                    }
                    cJSON_Delete(root);
                }
                ssd1306_draw_string(&oled_dev, 0, 7, "SYNC: DONE    ");
            } else {
                ESP_LOGE(TAG, "Sync: Fehler beim Abrufen der Ger&auml;teliste.");
                ssd1306_draw_string(&oled_dev, 0, 4, "SYNC: ERROR ");
            }
            // Aktuell alle 60 Sek poller, später Push/SSE
            vTaskDelay(pdMS_TO_TICKS(60000)); 
        } else {
            ssd1306_draw_string(&oled_dev, 0, 2, "CLOUD: LOGIN");
            // Noch nicht eingeloggt, alle 5 Sek prüfen
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
}

static void start_webserver() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.stack_size = 8192; // Doppelter Stack für lange URLs
    httpd_handle_t server = NULL;

    ESP_LOGI(TAG, "Starte Onboarding Webserver...");
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_get_handler };
        httpd_register_uri_handler(server, &index_uri);

        httpd_uri_t login_uri = { .uri = "/login", .method = HTTP_GET, .handler = login_get_handler };
        httpd_register_uri_handler(server, &login_uri);

        httpd_uri_t callback_uri = { .uri = "/callback", .method = HTTP_GET, .handler = callback_get_handler };
        httpd_register_uri_handler(server, &callback_uri);

        httpd_uri_t miele_logo_uri = { .uri = "/assets/miele.svg", .method = HTTP_GET, .handler = miele_svg_handler };
        httpd_register_uri_handler(server, &miele_logo_uri);

        httpd_uri_t matter_logo_uri = { .uri = "/assets/matter.svg", .method = HTTP_GET, .handler = matter_svg_handler };
        httpd_register_uri_handler(server, &matter_logo_uri);
        
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, not_found_handler);

        ESP_LOGI(TAG, "Onboarding Webserver läuft auf Port 80");
    } else {
        ESP_LOGE(TAG, "Webserver konnte nicht gestartet werden!");
    }
}

extern "C" void app_main()
{
    /* 1. NVS Initialisierung */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /* 1.5 OLED & I2C Initialisierung */
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 8,
        .scl_io_num = 9,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = { .clk_speed = 400000 }
    };
    i2c_param_config(I2C_NUM_0, &i2c_conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

    ssd1306_init(&oled_dev, I2C_NUM_0, 0x3C);
    ssd1306_clear(&oled_dev);
    ssd1306_draw_string(&oled_dev, 0, 0, "MIELE BRIDGE");
    ssd1306_draw_string(&oled_dev, 0, 2, "BOOTING...");

    /* 1. Basis-Infrastruktur */
    ssd1306_draw_string(&oled_dev, 0, 2, "INIT NET...     ");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // WLAN Power Save deaktivieren für maximale Pairing-Stabilität
    esp_wifi_set_ps(WIFI_PS_NONE);

    ssd1306_draw_string(&oled_dev, 0, 2, "INIT MIELE API..");
    miele::api::config_t miele_config;
    miele_config.client_id = MIELE_CLIENT_ID;
    miele_config.client_secret = MIELE_CLIENT_SECRET;
    miele::api::init(miele_config);

    ssd1306_draw_string(&oled_dev, 0, 2, "CREATE NODE...  ");
    node::config_t node_config;
    node_t *node = node::create(&node_config, NULL, NULL);

    ssd1306_draw_string(&oled_dev, 0, 2, "CREATE ENDP...  ");
    aggregator::config_t aggregator_config;
    endpoint_t *aggregator_endpoint = aggregator::create(node, &aggregator_config, ENDPOINT_FLAG_NONE, NULL);
    (void)aggregator_endpoint; // Unused for now
    
    dish_washer::config_t dishwasher_config;
    endpoint_t *dishwasher_endpoint = dish_washer::create(node, &dishwasher_config, ENDPOINT_FLAG_NONE, NULL);
    dishwasher_endpoint_id = endpoint::get_id(dishwasher_endpoint);
    (void)dishwasher_endpoint;

    /* 5. Onboarding Webserver & mDNS */
    ssd1306_draw_string(&oled_dev, 0, 2, "NET CONFIG...   ");
    mdns_init();
    mdns_hostname_set("miele-bridge");
    start_webserver();

    /* 7. Start Matter Stack & Console */
    ssd1306_draw_string(&oled_dev, 0, 2, "MATTER START... ");
    
    err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ssd1306_draw_string(&oled_dev, 0, 2, "MATTER ERROR!   ");
    } else {
        esp_matter::console::init(); // Konsole für Befehle aktivieren
        is_commissioned = chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned();
        
        if (!is_commissioned) {
            ssd1306_clear(&oled_dev);
            draw_qr_code();
            
            ESP_LOGI(TAG, "==============================================");
            ESP_LOGI(TAG, "   MATTER IDENTITY FORCED BY CONFIG: ");
            ESP_LOGI(TAG, "   PIN: 12345678");
            ESP_LOGI(TAG, "   DISCRIMINATOR: 1234");
            ESP_LOGI(TAG, "==============================================");
            
            // Jetzt lassen wir den Stack die offizielle Wahrheit sprechen:
            chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();
        }
    }

    /* 8. Miele Sync Task starten */
    xTaskCreate(miele_sync_task, "miele_sync", 8192, NULL, 5, NULL);

    ESP_LOGI(TAG, "Matter Stack läuft. Miele Cloud Sync gestartet.");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "Bridge Heartbeat (Matter + Miele)...");
    }
}
