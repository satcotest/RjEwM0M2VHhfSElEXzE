#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Include the necessary headers
#include "ups_hid_reports.h"
#include "ups_data.h"

// Mock the global variables for testing
ups_present_status_t g_power_summary_present_status = {
    .ac_present = true,
    .charging = true,
    .discharging = false,
    .fully_charged = false,
    .need_replacement = false,
    .below_remaining_capacity_limit = false,
    .battery_present = true,
    .overload = false,
    .shutdown_imminent = false,
};

ups_summary_t g_power_summary = {
    .rechargeable = true,
    .capacity_mode = 2U,
    .design_capacity = 100U,
    .full_charge_capacity = 100U,
    .warning_capacity_limit = 20U,
    .remaining_capacity_limit = 10U,
    .i_device_chemistry = 0x05U,
    .capacity_granularity_1 = 1U,
    .capacity_granularity_2 = 1U,
    .i_manufacturer_2bit = 1U,
    .i_product_2bit = 2U,
    .i_serial_number_2bit = 3U,
    .i_name_2bit = 2U,
};

ups_battery_t g_battery = {
    .battery_voltage = 0,
    .battery_current = 0,
    .config_voltage = 0,
    .run_time_to_empty_s = 3600,
    .remaining_time_limit_s = 120,
    .temperature = 0,
    .manufacturer_date = 0,
    .remaining_capacity = 50,
};

ups_input_t g_input = {
    .voltage = 0,
    .frequency = 0,
    .config_voltage = 0,
    .low_voltage_transfer = 0,
    .high_voltage_transfer = 0,
};

ups_output_t g_output = {
    .percent_load = 0,
    .config_active_power = 0,
    .config_voltage = 0,
    .voltage = 0,
    .current = 0,
    .frequency = 0,
};

int main() {
    uint8_t buffer[32];
    uint16_t len;
    
    // Test Input Report
    printf("Testing Input Report (Report ID 1):\n");
    len = build_hid_input_report(REPORT_ID_POWER_SUMMARY, buffer, sizeof(buffer));
    printf("Input Report length: %d\n", len);
    printf("Input Report data:");
    for (int i = 0; i < len; i++) {
        printf(" 0x%02X", buffer[i]);
    }
    printf("\n");
    
    // Test Feature Report
    printf("\nTesting Feature Report (Report ID 1):\n");
    len = build_hid_feature_report(REPORT_ID_POWER_SUMMARY, buffer, sizeof(buffer));
    printf("Feature Report length: %d\n", len);
    printf("Feature Report data:");
    for (int i = 0; i < len; i++) {
        printf(" 0x%02X", buffer[i]);
    }
    printf("\n");
    
    // Check specific fields
    printf("\nChecking specific fields:\n");
    printf("Expected remaining_capacity: 50 (0x32)\n");
    printf("Expected capacity_mode: 2 (0x02)\n");
    
    return 0;
}
