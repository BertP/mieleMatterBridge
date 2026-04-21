#include "miele_matter_mapping.h"
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_cluster.h>

static const char *TAG = "MIELE_MAPPING";

using namespace esp_matter;
using namespace esp_matter::cluster;

namespace miele {
namespace mapping {

void update_dishwasher_attributes(uint16_t endpoint_id, cJSON* device_json) {
    if (!device_json) return;

    // 1. Status Mapping
    cJSON* state = cJSON_GetObjectItem(device_json, "state");
    if (!state) return;

    cJSON* status = cJSON_GetObjectItem(state, "status");
    if (status) {
        cJSON* value_raw = cJSON_GetObjectItem(status, "value_raw");
        if (value_raw) {
            int miele_status = value_raw->valueint;
            uint8_t matter_op_state = 0; // Stopped

            switch (miele_status) {
                case 5: matter_op_state = 0x01; break; // Running
                case 6: matter_op_state = 0x02; break; // Paused
                case 7: matter_op_state = 0x01; break; // Let's use Running for End as test? No, 0=Stopped
                case 2: matter_op_state = 0x00; break; // Standby
                case 8: matter_op_state = 0x03; break; // Error
                default: matter_op_state = 0x00;
            }

            // Report to Matter (0x0060 = OperationalState, 0x0004 = CurrentOperationalState)
            esp_matter_attr_val_t val = esp_matter_uint8(matter_op_state);
            esp_matter::attribute::report(endpoint_id, 0x0060, 0x0004, &val);
            
            ESP_LOGI(TAG, "Dishwasher Status: Miele(%d) Matter(%d) on EP(%u)", 
                     (int)miele_status, (int)matter_op_state, (unsigned int)endpoint_id);
        }
    }

    // 2. Remaining Time Mapping
    cJSON* remaining = cJSON_GetObjectItem(state, "remainingTime");
    if (remaining && cJSON_IsArray(remaining) && cJSON_GetArraySize(remaining) >= 2) {
        cJSON* h = cJSON_GetArrayItem(remaining, 0);
        cJSON* m = cJSON_GetArrayItem(remaining, 1);
        if (h && m) {
            uint32_t total_seconds = (h->valueint * 3600) + (m->valueint * 60);
            ESP_LOGI(TAG, "Dishwasher Time: %u seconds", (unsigned int)total_seconds);
        }
    }
}

} // namespace mapping
} // namespace miele
