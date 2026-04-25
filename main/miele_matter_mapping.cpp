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

    cJSON* state = cJSON_GetObjectItem(device_json, "state");
    if (!state) return;

    // 1. Power State Mapping (OnOff Cluster: 0x0006, Attr: 0x0000)
    cJSON* status = cJSON_GetObjectItem(state, "status");
    bool is_on = false;
    if (status) {
        cJSON* value_raw = cJSON_GetObjectItem(status, "value_raw");
        if (value_raw) {
            // Miele Status: 1=Off, 2=Standby, 3=Normal, 4=Finished, 5=Running, etc.
            int miele_status = value_raw->valueint;
            is_on = (miele_status != 1); 
            
            esp_matter_attr_val_t on_off_val = esp_matter_bool(is_on);
            esp_matter::attribute::report(endpoint_id, 0x0006, 0x0000, &on_off_val);
            
            // 2. Operational State Mapping (OperationalState Cluster: 0x0060, Attr: 0x0004)
            uint8_t matter_op_state = 0; // 0x00 = Stopped
            switch (miele_status) {
                case 5: matter_op_state = 0x01; break; // 0x01 = Running
                case 6: matter_op_state = 0x02; break; // 0x02 = Paused
                case 4: 
                case 7: matter_op_state = 0x00; break; // 0x00 = Stopped
                case 8: matter_op_state = 0x03; break; // 0x03 = Error
                default: matter_op_state = 0x00;
            }

            esp_matter_attr_val_t op_val = esp_matter_uint8(matter_op_state);
            esp_matter::attribute::report(endpoint_id, 0x0060, 0x0004, &op_val);
            
            ESP_LOGI(TAG, "Dishwasher State updated: Power(%s) OpState(%d) on EP(%u)", 
                     is_on ? "ON" : "OFF", (int)matter_op_state, (unsigned int)endpoint_id);
        }
    }

    // 3. Dishwasher Mode Mapping (DishwasherMode Cluster: 0x0059, Attr: 0x0000)
    cJSON* program = cJSON_GetObjectItem(state, "programID");
    if (program && cJSON_IsNumber(program)) {
        int miele_program = program->valueint;
        // Matter modes are typically defined in the XML model. For test purposes, we map directly.
        uint8_t matter_mode = (uint8_t)miele_program;
        
        esp_matter_attr_val_t mode_val = esp_matter_uint8(matter_mode);
        esp_matter::attribute::report(endpoint_id, 0x0059, 0x0000, &mode_val);
        
        ESP_LOGI(TAG, "Dishwasher Mode updated: MieleProgram(%d) -> MatterMode(%d)", 
                 miele_program, (int)matter_mode);
    }

    // 4. Remaining Time Mapping (OperationalState Cluster: 0x0060, Attr: 0x0002 - CountdownTime)
    cJSON* remaining = cJSON_GetObjectItem(state, "remainingTime");
    if (remaining && cJSON_IsArray(remaining) && cJSON_GetArraySize(remaining) >= 2) {
        cJSON* h = cJSON_GetArrayItem(remaining, 0);
        cJSON* m = cJSON_GetArrayItem(remaining, 1);
        if (h && m) {
            uint32_t total_seconds = (h->valueint * 3600) + (m->valueint * 60);
            
            esp_matter_attr_val_t countdown_val = esp_matter_uint32(total_seconds);
            esp_matter::attribute::report(endpoint_id, 0x0060, 0x0002, &countdown_val);
            
            ESP_LOGI(TAG, "Dishwasher Countdown: %u seconds remaining", (unsigned int)total_seconds);
        }
    }
}

} // namespace mapping
} // namespace miele
