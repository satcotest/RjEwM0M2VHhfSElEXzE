#include "ups_hid_reports.h"
#include "ups_hid_config.h"
#include <string.h>

// 全局配置
extern ups_hid_config_t g_ups_config;

// 报告大小定义
#define INPUT_REPORT_SIZE       8u
#define FEATURE_REPORT_SIZE     20u

// 打包 PresentStatus 位字段 (5位状态 + 3位填充 = 8位)
static uint8_t pack_present_status(void)
{
    return (uint8_t)(
        (g_ups_config.status1.bits.ac_present      << 0) |
        (g_ups_config.status1.bits.charging        << 1) |
        (g_ups_config.status1.bits.discharging     << 2) |
        (g_ups_config.status1.bits.fully_charged   << 3) |
        (g_ups_config.status1.bits.battery_present << 4)
    );
}

// ============================================================================
// Report ID 1: Power Summary Input Report (8字节)
// [RemainingCapacity(1)] [RunTimeToEmpty(2)] [Voltage(2)] [Current(2)] [PresentStatus(1)]
// ============================================================================
static uint16_t build_power_summary_input(uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen < INPUT_REPORT_SIZE))
    {
        return 0u;
    }

    buffer[0] = g_ups_config.remaining_capacity;
    buffer[1] = (uint8_t)(g_ups_config.run_time_to_empty & 0xFFu);
    buffer[2] = (uint8_t)(g_ups_config.run_time_to_empty >> 8);
    buffer[3] = (uint8_t)(g_ups_config.voltage & 0xFFu);
    buffer[4] = (uint8_t)(g_ups_config.voltage >> 8);
    buffer[5] = (uint8_t)(g_ups_config.current & 0xFFu);
    buffer[6] = (uint8_t)((int16_t)g_ups_config.current >> 8);
    buffer[7] = pack_present_status();

    return INPUT_REPORT_SIZE;
}

// ============================================================================
// Report ID 1: Power Summary Feature Report (20字节)
// ============================================================================
static uint16_t build_power_summary_feature(uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen < FEATURE_REPORT_SIZE))
    {
        return 0u;
    }

    // 前9字节：字符串索引(5) + 模式属性(2) + 容量限制(2)
    buffer[0] = g_ups_config.i_manufacturer;
    buffer[1] = g_ups_config.i_product;
    buffer[2] = g_ups_config.i_serial;
    buffer[3] = g_ups_config.i_name;
    buffer[4] = g_ups_config.i_device_chemistry;
    buffer[5] = g_ups_config.capacity_mode;
    buffer[6] = g_ups_config.rechargeable ? 1u : 0u;
    buffer[7] = g_ups_config.warning_capacity_limit;
    buffer[8] = g_ups_config.remaining_capacity_limit;

    // 中间7字节：容量值(3) + 时间值(4)
    buffer[9]  = g_ups_config.remaining_capacity;
    buffer[10] = g_ups_config.full_charge_capacity;
    buffer[11] = g_ups_config.design_capacity;
    buffer[12] = (uint8_t)(g_ups_config.run_time_to_empty & 0xFFu);
    buffer[13] = (uint8_t)(g_ups_config.run_time_to_empty >> 8);
    buffer[14] = (uint8_t)(g_ups_config.remaining_time_limit & 0xFFu);
    buffer[15] = (uint8_t)(g_ups_config.remaining_time_limit >> 8);

    // 后4字节：粒度(2) + 状态(1) + 填充(1)
    buffer[16] = g_ups_config.capacity_granularity_1;
    buffer[17] = g_ups_config.capacity_granularity_2;
    buffer[18] = pack_present_status();
    buffer[19] = 0u; // 填充

    return FEATURE_REPORT_SIZE;
}

// ============================================================================
// 公共 API
// ============================================================================

uint16_t build_hid_input_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen)
{
    if (report_id == REPORT_ID_POWER_SUMMARY)
    {
        return build_power_summary_input(buffer, reqlen);
    }
    return 0u;
}

uint16_t build_hid_feature_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen)
{
    if (report_id == REPORT_ID_POWER_SUMMARY)
    {
        return build_power_summary_feature(buffer, reqlen);
    }
    return 0u;
}
