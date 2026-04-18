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
    .voltage = 22000,  // 220V
    .frequency = 5000, // 50Hz
    .config_voltage = 22000,
    .low_voltage_transfer = 19000,
    .high_voltage_transfer = 24000,
};

ups_output_t g_output = {
    .percent_load = 50,
    .config_active_power = 1000,
    .config_voltage = 22000,
    .voltage = 22000,
    .current = 2000,
    .frequency = 5000,
};

// Function to print report data
void print_report(const char* name, uint8_t* buffer, uint16_t len) {
    printf("%s length: %d\n", name, len);
    printf("Data:");
    for (int i = 0; i < len; i++) {
        printf(" 0x%02X", buffer[i]);
    }
    printf("\n");
}

int main() {
    uint8_t buffer[32];
    uint16_t len;
    
    // Test Input Report
    printf("Testing Input Report (Report ID 1):\n");
    len = build_hid_input_report(REPORT_ID_POWER_SUMMARY, buffer, sizeof(buffer));
    print_report("Input Report", buffer, len);
    
    // Test Feature Reports
    printf("\nTesting Feature Reports:\n");
    
    // Power Summary Feature Report
    printf("\nPower Summary Feature Report (Report ID 1):\n");
    len = build_hid_feature_report(REPORT_ID_POWER_SUMMARY, buffer, sizeof(buffer));
    print_report("Feature Report", buffer, len);
    
    // Input Feature Report
    printf("\nInput Feature Report (Report ID 2):\n");
    len = build_hid_feature_report(REPORT_ID_INPUT, buffer, sizeof(buffer));
    print_report("Feature Report", buffer, len);
    
    // Output Feature Report
    printf("\nOutput Feature Report (Report ID 3):\n");
    len = build_hid_feature_report(REPORT_ID_OUTPUT, buffer, sizeof(buffer));
    print_report("Feature Report", buffer, len);
    
    // Battery Feature Report
    printf("\nBattery Feature Report (Report ID 4):\n");
    len = build_hid_feature_report(REPORT_ID_BATTERY, buffer, sizeof(buffer));
    print_report("Feature Report", buffer, len);
    
    // Check specific fields in Power Summary Feature Report
    printf("\nChecking Power Summary Feature Report fields:\n");
    len = build_hid_feature_report(REPORT_ID_POWER_SUMMARY, buffer, sizeof(buffer));
    if (len >= 17) {
        printf("warning_capacity_limit: 0x%02X (expected 0x14)\n", buffer[0]);
        printf("remaining_capacity_limit: 0x%02X (expected 0x0A)\n", buffer[1]);
        printf("remaining_capacity: 0x%02X (expected 0x32)\n", buffer[2]);
        printf("run_time_to_empty_s: 0x%02X%02X (expected 0x0E10)\n", buffer[4], buffer[3]);
        printf("remaining_time_limit_s: 0x%02X%02X (expected 0x0078)\n", buffer[6], buffer[5]);
        printf("i_device_chemistry: 0x%02X (expected 0x05)\n", buffer[7]);
        printf("capacity_mode: 0x%02X (expected 0x02)\n", buffer[8]);
        printf("design_capacity: 0x%02X (expected 0x64)\n", buffer[9]);
        printf("full_charge_capacity: 0x%02X (expected 0x64)\n", buffer[10]);
        printf("rechargeable_and_padding: 0x%02X (expected 0x01)\n", buffer[11]);
        printf("capacity_granularity_1: 0x%02X (expected 0x01)\n", buffer[12]);
        printf("capacity_granularity_2: 0x%02X (expected 0x01)\n", buffer[13]);
        printf("i_strings_packed: 0x%02X (expected 0x6A)\n", buffer[14]);
        printf("present_status_bits: 0x%02X%02X (expected 0x0003)\n", buffer[16], buffer[15]);
    }
    
    return 0;
}
