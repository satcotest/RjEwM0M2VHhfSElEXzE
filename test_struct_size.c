#include <stdint.h>
#include <stdbool.h>

#define TU_ATTR_PACKED __attribute__((packed))

typedef struct TU_ATTR_PACKED
{
    uint8_t warning_capacity_limit;
    uint8_t remaining_capacity_limit;
    uint8_t remaining_capacity;
    uint16_t run_time_to_empty_s;
    uint16_t remaining_time_limit_s;
    uint8_t i_device_chemistry;
    uint8_t capacity_mode;
    uint8_t full_charge_capacity;
    uint8_t design_capacity;
    uint8_t rechargeable_and_padding;
    uint8_t capacity_granularity_1;
    uint8_t capacity_granularity_2;
    uint8_t i_strings_packed;
    uint16_t present_status_bits;
} ups_report_power_summary_feature_t;

int main() {
    return sizeof(ups_report_power_summary_feature_t);
}