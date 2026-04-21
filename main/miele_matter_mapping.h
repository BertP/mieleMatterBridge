#pragma once

#include <esp_matter.h>
#include <cJSON.h>

namespace miele {
namespace mapping {

/**
 * @brief Maps Miele appliance status to Matter Dishwasher cluster attributes
 * 
 * @param endpoint_id The Matter endpoint ID for the dishwasher
 * @param device_json The JSON object for a single Miele device
 */
void update_dishwasher_attributes(uint16_t endpoint_id, cJSON* device_json);

} // namespace mapping
} // namespace miele
