#pragma once

#include "esp_err.h"
#include <string>
#include <vector>

namespace miele {
namespace api {

/**
 * @brief Miele Cloud Credentials & State
 */
struct config_t {
    std::string client_id;
    std::string client_secret;
    std::string access_token;
    std::string refresh_token;
};

/**
 * @brief Appliance State Summary
 */
struct appliance_status_t {
    std::string device_id;
    int status_id;
    int program_id;
    int remaining_time_mins;
    bool is_on;
};

/**
 * @brief Initialize the Miele Client
 */
esp_err_t init(const config_t &config);

esp_err_t get_devices(std::string &response);

bool is_authenticated();

/**
 * @brief Tauscht den Authorization Code gegen Tokens
 */
esp_err_t exchange_code(const std::string &code, const std::string &redirect_uri);

/**
 * @brief Refresh the Access Token using the Refresh Token
 */
esp_err_t refresh_token();

/**
 * @brief Get the status of a specific dishwasher
 * 
 * @param device_id The serial number of the appliance
 * @param out_status Pointer to status struct to fill
 */
esp_err_t get_appliance_status(const std::string &device_id, appliance_status_t *out_status);

/**
 * @brief Start the Server-Sent Events (SSE) listener for real-time updates
 */
esp_err_t start_sse_listener();

/**
 * @brief Löscht die Sitzung (Tokens) lokal und im NVS
 */
esp_err_t logout();

} // namespace api
} // namespace miele
