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
#include "led_strip.h"
#include "driver/rmt.h"
#include "driver/i2c.h"
#include "miele_matter_mapping.h"
#include "CHIPProjectConfig.h"
#include "ui_manager.h"
#include "esp_wifi.h"

// QR Code generation
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include "qrcodegen.hpp"

static const char *TAG = "MIELE_BRIDGE";
#define APP_VERSION "2.3.9"
#define APP_CODENAME "Zen Harmony Pro"

enum class SystemStatus {
    PAIRING,
    CONNECTED,
    MIELE_AUTH_PENDING,
    ERROR
};


// Forward Declarations
static void start_webserver();
static void miele_sync_task(void *pvParameters);
static void start_services();

// Custom Device Info Provider to fix names in Google Home / Matter
class CustomDeviceInstanceInfoProvider : public chip::DeviceLayer::DeviceInstanceInfoProvider {
public:
    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override {
        chip::Platform::CopyString(buf, bufSize, CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override {
        vendorId = CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override {
        chip::Platform::CopyString(buf, bufSize, CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductId(uint16_t & productId) override {
        productId = CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override {
        hardwareVersion = 1;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override {
        chip::Platform::CopyString(buf, bufSize, "v1.0");
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override {
        chip::Platform::CopyString(buf, bufSize, "MIELE-MATTER-001");
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override {
        year = 2024; month = 4; day = 24;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(chip::MutableByteSpan & uniqueIdSpan) override {
        static const uint8_t uniqueId[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };
        return chip::CopySpanToMutableSpan(chip::ByteSpan(uniqueId), uniqueIdSpan);
    }
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override {
        chip::Platform::CopyString(buf, bufSize, "G7104-SCU");
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override {
        chip::Platform::CopyString(buf, bufSize, "https://www.miele.de");
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override {
        chip::Platform::CopyString(buf, bufSize, "Miele Dishwasher Bridge");
        return CHIP_NO_ERROR;
    }
};

static CustomDeviceInstanceInfoProvider gCustomDeviceInstanceInfoProvider;

#define HEARTBEAT_LED_GPIO 48

static ssd1306_t oled_dev;
static led_strip_t *led_strip;
static uint16_t dishwasher_endpoint_id = 0;

static bool is_wifi_connected = false;
static bool is_commissioned = false;
static std::string last_dishwasher_status = "IDLE";
static int last_dishwasher_progress = 0;

// Marker for embedded assets
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t miele_svg_start[]  asm("_binary_miele_svg_start");
extern const uint8_t miele_svg_end[]    asm("_binary_miele_svg_end");
extern const uint8_t matter_svg_start[] asm("_binary_matter_svg_start");
extern const uint8_t matter_svg_end[]   asm("_binary_matter_svg_end");

using namespace esp_matter;
using namespace esp_matter::endpoint;
using namespace esp_matter::cluster;

// Credentials are now managed via Kconfig (CONFIG_MIELE_CLIENT_ID / CONFIG_MIELE_CLIENT_SECRET)
#define MIELE_CLIENT_ID CONFIG_MIELE_CLIENT_ID
#define MIELE_CLIENT_SECRET CONFIG_MIELE_CLIENT_SECRET

// static bool is_commissioned = false; // Moved up

// 32x32 Checkmark Bitmap (4 pages high)
static const uint8_t checkmark_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0x70, 0x38, 0x1C, 0x0E,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0,
    0xE0, 0xF0, 0xF8, 0xFC, 0x7E, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void start_services() {
    static bool services_started = false;
    if (services_started) return;
    
    ESP_LOGI(TAG, "Starte System-Dienste (Webserver & mDNS)...");
    
    // mDNS Initialisierung
    esp_err_t mdns_err = mdns_init();
    if (mdns_err == ESP_OK) {
        mdns_hostname_set("miele-bridge");
        mdns_instance_name_set("Miele Matter Bridge");
        mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
    }

    start_webserver();
    
    // Miele Sync Task falls noch nicht gestartet
    xTaskCreate(miele_sync_task, "miele_sync", 8192, NULL, 5, NULL);
    
    services_started = true;
}

static void refresh_display() {
    if (!is_commissioned) return; 
    
    // Get IP Address safely
    char ip_addr[IP4ADDR_STRLEN_MAX] = "0.0.0.0";
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif) {
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            esp_ip4addr_ntoa(&ip_info.ip, ip_addr, IP4ADDR_STRLEN_MAX);
        }
    }

    ui::draw_dashboard(&oled_dev, APP_VERSION, 
                      is_wifi_connected, 
                      is_commissioned, 
                      miele::api::is_authenticated(), 
                      ip_addr);
}

static void draw_success_screen() {
    ssd1306_clear(&oled_dev);
    char header[32];
    snprintf(header, sizeof(header), "MIELE BRIDGE %s", APP_VERSION);
    ssd1306_draw_string(&oled_dev, (128 - strlen(header)*8)/2, 0, header);
    
    // Draw Checkmark in center (32x32 is 4 pages)
    ssd1306_draw_bitmap(&oled_dev, 48, 2, 32, 4, checkmark_bitmap);
    
    ssd1306_draw_string(&oled_dev, 32, 7, "SYSTEM READY");
    vTaskDelay(pdMS_TO_TICKS(3000));
    refresh_display();
}


static void draw_qr_code() {
    using namespace qrcodegen;
    
    // 1. Get Setup Info from Matter Stack
    uint32_t setup_passcode = 0;
    uint16_t setup_discriminator = 0;
    uint16_t vid = 0, pid = 0;
    
    // Safety checks for providers
    auto* commissionable_provider = chip::DeviceLayer::GetCommissionableDataProvider();
    auto* instance_info_provider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();

    if (commissionable_provider) {
        (void)commissionable_provider->GetSetupPasscode(setup_passcode);
        (void)commissionable_provider->GetSetupDiscriminator(setup_discriminator);
    } else {
        ESP_LOGW(TAG, "CommissionableDataProvider not ready, using defaults");
        setup_passcode = 20202021;
        setup_discriminator = 3840;
    }
    
    if (instance_info_provider) {
        (void)instance_info_provider->GetVendorId(vid);
        (void)instance_info_provider->GetProductId(pid);
    } else {
        ESP_LOGW(TAG, "DeviceInstanceInfoProvider not ready, using defaults");
        vid = 0x120E;
        pid = 0x0001;
    }
    
    // 2. Generate Setup Payload String
    chip::SetupPayload payload;
    payload.version = 0;
    payload.vendorID = vid;
    payload.productID = pid;
    payload.commissioningFlow = chip::CommissioningFlow::kStandard;
    payload.rendezvousInformation.SetValue(chip::RendezvousInformationFlag::kBLE);
    payload.setUpPINCode = setup_passcode;
    payload.discriminator.SetLongValue(setup_discriminator);
    
    std::string qr_url;
    // 3. Generate QR Payload String
    (void)chip::QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(qr_url);
    ESP_LOGI(TAG, "Generated QR Payload: %s", qr_url.c_str());

    // 3. Generate QR Matrix
    QrCode qr = QrCode::encodeText(qr_url.c_str(), QrCode::Ecc::LOW);
    int size = qr.getSize();
    
    // 4. Render to OLED (centered, 2x2 scaling if possible)
    // Clear screen first
    ssd1306_clear(&oled_dev);
    char header[20];
    snprintf(header, sizeof(header), "PAIRING %s", APP_VERSION);
    ssd1306_draw_string(&oled_dev, 0, 0, header);
    
    // Calculate scaling and offsets
    int scale = (size * 2 <= 56) ? 2 : 1; // max height is 64, minus page 0 (8px) = 56px
    uint8_t x_offset = (128 - (size * scale)) / 2;
    uint8_t y_page_start = 1; // Start below "PAIRING MODE"
    
    for (int p = 0; p < 7; p++) { // Pages 1 to 7
        uint8_t current_page = y_page_start + p;
        if (current_page > 7) break;
        
        ssd1306_send_cmd(&oled_dev, 0xB0 + current_page);
        ssd1306_send_cmd(&oled_dev, x_offset & 0x0F);
        ssd1306_send_cmd(&oled_dev, 0x10 | (x_offset >> 4));
        
        uint8_t page_data[128] = {0};
        
        for (int qx = 0; qx < size; qx++) {
            uint8_t byte = 0;
            // For each bit in the page (8 vertical pixels)
            for (int bit = 0; bit < 8; bit++) {
                int screen_y = p * 8 + bit;
                int qr_y = screen_y / scale;
                int qr_x = qx;
                
                if (qr_y < size && qr.getModule(qr_x, qr_y)) {
                    byte |= (1 << bit);
                }
            }
            
            for (int s = 0; s < scale; s++) {
                page_data[qx * scale + s] = byte;
            }
        }
        ssd1306_send_data(&oled_dev, page_data, size * scale);
    }
    
    // Also log manual code
    ESP_LOGI(TAG, "Manual Pairing PIN: %lu", setup_passcode);
}

/**
 * @brief Geballte Matter-Events (Pairing-Status etc.)
 */
static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg) {
    switch (event->Type) {
        case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged: {
            ESP_LOGI(TAG, "IP-Adresse erhalten oder geändert. Warte kurz für mDNS Stabilität...");
            vTaskDelay(pdMS_TO_TICKS(1000)); // Gedenksekunde für mDNS Stack
            char ip_buf[32];
            esp_netif_ip_info_t ip_info;
            esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
            if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
                esp_ip4addr_ntoa(&ip_info.ip, ip_buf, sizeof(ip_buf));
                ESP_LOGI(TAG, "IP-Adresse: %s", ip_buf);
                is_wifi_connected = true;
                refresh_display();
            } else {
                is_wifi_connected = false;
                refresh_display();
            }
            break;
        }
        case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
            ESP_LOGI(TAG, "Pairing erfolgreich abgeschlossen! 🎉");
            is_commissioned = true;
            start_services(); // Sofort Webserver & mDNS starten
            draw_success_screen();
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
    char host[64];
    if (httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK) {
        strcpy(host, "miele-bridge.local");
    }

    // Hier bauen wir die Miele Auth URL zusammen
    char redirect_url[512];
    snprintf(redirect_url, sizeof(redirect_url), 
        "https://auth.domestic.miele-iot.com/partner/realms/mcs/protocol/openid-connect/auth?"
        "client_id=%s&response_type=code&scope=openid%%20mcs_thirdparty_read%%20mcs_thirdparty_write&redirect_uri=http://%s/callback",
        MIELE_CLIENT_ID, host);
    
    ESP_LOGI(TAG, "Redirecting to Miele Auth: %s", redirect_url);
    
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
            ESP_LOGI(TAG, "Query-String: %s", buf);
            if (httpd_query_key_value(buf, "code", code, sizeof(code)) == ESP_OK) {
                ESP_LOGI(TAG, "Code erfolgreich extrahiert (Länge: %d)", strlen(code));
            } else {
                ESP_LOGW(TAG, "Parameter 'code' nicht im Query-String gefunden.");
            }
        }
        free(buf);
    }

    if (strlen(code) > 0) {
        ESP_LOGI(TAG, "Auth Code erhalten, starte Exchange...");
        
        // Dynamische redirect_uri basierend auf dem Host-Header
        char host[64];
        if (httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK) {
            strcpy(host, "miele-bridge.local");
        }
        std::string redirect_uri = "http://";
        redirect_uri += host;
        redirect_uri += "/callback";
        
        ESP_LOGI(TAG, "Benutze redirect_uri: %s", redirect_uri.c_str());
        esp_err_t exchange_err = miele::api::exchange_code(code, redirect_uri);
        if (exchange_err == ESP_OK) {
            ESP_LOGI(TAG, "Miele Cloud Login erfolgreich! 🎉");
            const char *success_html = 
                "<html><head><meta http-equiv='refresh' content='2;url=/'></head>"
                "<body style='background:#121212;color:white;display:flex;justify-content:center;align-items:center;height:100vh;font-family:sans-serif;'>"
                "<div style='text-align:center;'><h1>Verbunden!</h1><p>Leite weiter zum Dashboard...</p></div>"
                "</body></html>";
            httpd_resp_send(req, success_html, HTTPD_RESP_USE_STRLEN);
        } else {
            ESP_LOGE(TAG, "Exchange Code fehlgeschlagen: 0x%x", exchange_err);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Miele API Exchange failed");
        }
    } else {
        ESP_LOGW(TAG, "Kein Auth Code im Callback gefunden!");
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing code parameter");
    }
    return ESP_OK;
}

/**
 * @brief 404 Handler zum Debuggen von falschen URIs
 */
static esp_err_t not_found_handler(httpd_req_t *req, httpd_err_code_t error) {
    ESP_LOGW(TAG, "Nicht gefundene URI angefragt: %s", req->uri);
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "URI nicht gefunden");
    return ESP_OK;
}

/**
 * @brief GET /api/status Handler: Liefert Systemstatus als JSON
 */
static esp_err_t status_get_handler(httpd_req_t *req) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "version", APP_VERSION);
    cJSON_AddBoolToObject(root, "commissioned", is_commissioned);
    cJSON_AddBoolToObject(root, "miele_auth", miele::api::is_authenticated());
    
    char ip_buf[32] = "0.0.0.0";
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
        esp_ip4addr_ntoa(&ip_info.ip, ip_buf, sizeof(ip_buf));
    }
    cJSON_AddStringToObject(root, "ip", ip_buf);

    const char *sys_info = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, sys_info, HTTPD_RESP_USE_STRLEN);
    
    free((void*)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

/**
 * @brief POST /api/reset Handler: Führt einen Werksreset durch
 */
static esp_err_t reset_post_handler(httpd_req_t *req) {
    ESP_LOGW(TAG, "FACTORY RESET TRIGGERED VIA WEB UI!");
    
    const char *response = "{\"status\":\"resetting\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    chip::Server::GetInstance().ScheduleFactoryReset();
    return ESP_OK;
}

/**
 * @brief Hintergrund-Task für den Heartbeat (LED-Puls)
 */
static void status_led_task(void *pvParameters) {
    while (1) {
        uint8_t r = 0, g = 0, b = 0;
        uint8_t brightness = 15; 

        // Echtzeit-Status abfragen
        bool wifi = is_wifi_connected;
        bool commissioned = chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned();
        bool authenticated = miele::api::is_authenticated();

        if (!wifi) {
            r = brightness; // Rot: Kein WLAN
        } else if (!commissioned) {
            b = brightness; // Blau: Pairing Modus
        } else if (!authenticated) {
            r = brightness; g = brightness / 2; // Orange: Miele Login fehlt
        } else {
            g = brightness; // Grün: Alles OK!
        }

        if (led_strip) {
            led_strip->set_pixel(led_strip, 0, r, g, b);
            led_strip->refresh(led_strip, 100);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static esp_err_t disconnect_post_handler(httpd_req_t *req) {
    miele::api::logout();
    ESP_LOGI(TAG, "Miele Account manuell getrennt.");
    httpd_resp_sendstr(req, "OK");
    return ESP_OK;
}

/**
 * @brief Hintergrund-Task für den Miele Cloud-Sync
 */
static void miele_sync_task(void *pvParameters) {
    ESP_LOGI(TAG, "Miele Sync Task gestartet.");
    bool was_commissioned = false;
    
    while (1) {
        bool current_commissioned = chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned();
        
        // Automatischer OLED-Refresh nach Pairing
        if (current_commissioned && !was_commissioned) {
            ESP_LOGI(TAG, "Pairing abgeschlossen. Aktualisiere OLED...");
            ssd1306_clear(&oled_dev);
            char header[20];
            snprintf(header, sizeof(header), "BRIDGE ONLINE %s", APP_VERSION);
            ssd1306_draw_string(&oled_dev, 0, 0, header);
            
            esp_netif_ip_info_t ip_info;
            esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info);
            char ip_str[32];
            snprintf(ip_str, sizeof(ip_str), "IP: " IPSTR, IP2STR(&ip_info.ip));
            ssd1306_draw_string(&oled_dev, 0, 3, ip_str);
            
            ssd1306_draw_string(&oled_dev, 0, 5, "Miele Login bereit");
            was_commissioned = true;
        }

        if (!current_commissioned) {
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        if (miele::api::is_authenticated()) {
            // Once commissioned, we don't draw debug info to OLED anymore (Zen Design)
            if (!is_commissioned) {
                ssd1306_draw_string(&oled_dev, 0, 2, "CLOUD: OK   ");
            }
            
            ESP_LOGI(TAG, "Sync: Frage Miele Ger&auml;teliste ab...");
            std::string response;
            if (miele::api::get_devices(response) == ESP_OK) {
                // ... logic to update Matter attributes (always needed) ...
                cJSON* root = cJSON_Parse(response.c_str());
                if (root) {
                    cJSON* device = NULL;
                    cJSON* target_dishwasher = NULL;
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
                        
                        // Update UI state
                        cJSON* state = cJSON_GetObjectItem(target_dishwasher, "state");
                        if (state) {
                            cJSON* status = cJSON_GetObjectItem(state, "status");
                            if (status) last_dishwasher_status = cJSON_GetObjectItem(status, "value_localized")->valuestring;
                            
                            cJSON* signal = cJSON_GetObjectItem(state, "signalInfo");
                            if (signal && cJSON_GetObjectItem(signal, "value_raw")->valueint > 0) {
                                last_dishwasher_status = "FINISHED";
                                last_dishwasher_progress = 100;
                            } else {
                                // Simple progress simulation if no direct progress field
                                last_dishwasher_progress = 50; // TODO: Calculate from remainingTime
                            }
                        }
                        refresh_display();
                    }
                    cJSON_Delete(root);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(30000)); 
        } else {
            refresh_display();
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

        httpd_uri_t status_uri = { .uri = "/api/status", .method = HTTP_GET, .handler = status_get_handler };
        httpd_register_uri_handler(server, &status_uri);

        httpd_uri_t reset_uri = { .uri = "/api/reset", .method = HTTP_POST, .handler = reset_post_handler };
        httpd_register_uri_handler(server, &reset_uri);

        httpd_uri_t disconnect_uri = {
            .uri      = "/api/disconnect",
            .method   = HTTP_POST,
            .handler  = disconnect_post_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &disconnect_uri);
        
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
        .master = { .clk_speed = 400000 },
        .clk_flags = 0
    };
    i2c_param_config(I2C_NUM_0, &i2c_conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

    ssd1306_init(&oled_dev, I2C_NUM_0, 0x3C);
    ssd1306_clear(&oled_dev);
    char boot_header[20];
    snprintf(boot_header, sizeof(boot_header), "MIELE %s", APP_VERSION);
    ssd1306_draw_string(&oled_dev, 0, 0, boot_header);
    ssd1306_draw_string(&oled_dev, 0, 2, "BOOTING...");

    // Heartbeat LED Initialisierung (WS2812 auf GPIO 48) - Version 1.x/2.x API
    rmt_config_t rmt_conf_pulse = RMT_DEFAULT_CONFIG_TX((gpio_num_t)HEARTBEAT_LED_GPIO, RMT_CHANNEL_0);
    rmt_conf_pulse.clk_div = 2;
    ::rmt_config(&rmt_conf_pulse);
    rmt_driver_install(rmt_conf_pulse.channel, 0, 0);

    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(1, (led_strip_dev_t)RMT_CHANNEL_0);
    led_strip = led_strip_new_rmt_ws2812(&strip_config);
    xTaskCreate(status_led_task, "status_led", 2048, NULL, 1, NULL);

    /* 1. Basis-Infrastruktur (Minimal für Pairing) */
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // mDNS und Webserver starten wir jetzt erst NACH dem Pairing, 
    // um den Fail-safe Timer (1s!) nicht zu gefährden.
    
    // WLAN Power Save wird später in app_main nach dem Stack-Start deaktiviert

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

    /* 5. Onboarding Webserver (Wird später bei Bedarf gestartet) */

    /* 7. Start Matter Stack & Console */
    ssd1306_draw_string(&oled_dev, 0, 2, "MATTER START... ");
    
    // Set custom info provider BEFORE starting Matter to fix naming
    chip::DeviceLayer::SetDeviceInstanceInfoProvider(&gCustomDeviceInstanceInfoProvider);
    
    // Check commissioning status
    is_commissioned = chip::DeviceLayer::ConfigurationMgr().IsFullyProvisioned();
    /* Matter start */
    err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ssd1306_draw_string(&oled_dev, 0, 2, "MATTER ERROR!   ");
    } else {
        esp_matter::console::init(); 
        
        // FORCE Miele Identity in Basic Information Cluster (Endpoint 0)
        // We do this AFTER start to ensure the clusters are fully ready
        esp_matter_attr_val_t val = esp_matter_uint16(0x120E);
        attribute::update(0, chip::app::Clusters::BasicInformation::Id, chip::app::Clusters::BasicInformation::Attributes::VendorID::Id, &val);

        val = esp_matter_uint16(0x0001);
        attribute::update(0, chip::app::Clusters::BasicInformation::Id, chip::app::Clusters::BasicInformation::Attributes::ProductID::Id, &val);

        val = esp_matter_char_str((char*)"Miele & Cie. KG", strlen("Miele & Cie. KG"));
        attribute::update(0, chip::app::Clusters::BasicInformation::Id, chip::app::Clusters::BasicInformation::Attributes::VendorName::Id, &val);
        
        val = esp_matter_char_str((char*)"Miele Matter Bridge", strlen("Miele Matter Bridge"));
        attribute::update(0, chip::app::Clusters::BasicInformation::Id, chip::app::Clusters::BasicInformation::Attributes::ProductName::Id, &val);

        val = esp_matter_char_str((char*)APP_VERSION, strlen(APP_VERSION));
        attribute::update(0, chip::app::Clusters::BasicInformation::Id, chip::app::Clusters::BasicInformation::Attributes::SoftwareVersionString::Id, &val);

        // WLAN Power Save erst hier deaktivieren, wenn der Stack (und damit Wifi) bereit ist
        esp_wifi_set_ps(WIFI_PS_NONE);

        if (!is_commissioned) {
            // ZEN SEQUENCE: QR Code einmalig zeichnen und dann Display-Updates pausieren
            vTaskDelay(pdMS_TO_TICKS(1000));
            ESP_LOGI(TAG, "Device not commissioned. Showing QR code once...");
            draw_qr_code();
            // Wir lassen den QR-Code stehen und starten keine Hintergrund-UI-Updates
        } else {
            ESP_LOGI(TAG, "Device already commissioned. Switching to Dashboard.");
            draw_success_screen();
        }
    }

    /* 8. Miele Sync & Webserver Task starten (nur wenn bereits commissioned) */
    if (is_commissioned) {
        start_services();
    }

    ESP_LOGI(TAG, "Miele Matter Bridge %s (%s) gestartet.", APP_VERSION, APP_CODENAME);
    ESP_LOGI(TAG, "Matter Stack läuft. Miele Cloud Sync gestartet.");

    /* 9. Factory Reset Button (GPIO 0 - Boot Button) */
    gpio_config_t btn_config = {
        .pin_bit_mask = (1ULL << GPIO_NUM_0),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&btn_config);

    // Einfacher Überwachungs-Task für den Reset-Knopf
    xTaskCreate([](void*){
        int hold_time = 0;
        while(1) {
            if (gpio_get_level(GPIO_NUM_0) == 0) {
                hold_time++;
                if (hold_time == 50) { // 5 Sekunden (50 * 100ms)
                    ESP_LOGW(TAG, "Factory Reset via Button ausgelöst!");
                    chip::Server::GetInstance().ScheduleFactoryReset();
                }
            } else {
                hold_time = 0;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }, "reset_btn_task", 2048, NULL, 1, NULL);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGI(TAG, "Bridge Heartbeat (Matter + Miele)...");
    }
}
