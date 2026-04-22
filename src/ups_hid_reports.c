#include "ups_hid_reports.h"
#include "ups_hid_config.h"
#include <string.h>

// Report ID 定义
#define REPORT_ID_POWER_SUMMARY 0x01

// 全局配置
extern ups_hid_config_t g_ups_config;

// 打包 PresentStatus 位字段 (5位状态 + 3位填充 = 8位)
static uint8_t pack_present_status(void)
{
    uint8_t bits = 0U;

    if (g_ups_config.status1.bits.ac_present)
        bits |= (1U << 0);   // bit 0: AC Present
    if (g_ups_config.status1.bits.charging)
        bits |= (1U << 1);   // bit 1: Charging
    if (g_ups_config.status1.bits.discharging)
        bits |= (1U << 2);   // bit 2: Discharging
    if (g_ups_config.status1.bits.fully_charged)
        bits |= (1U << 3);   // bit 3: Fully Charged
    if (g_ups_config.status1.bits.battery_present)
        bits |= (1U << 4);   // bit 4: Battery Present

    return bits;
}

// ============================================================================
// Report ID 1: Power Summary Input Report
// 格式: [RemainingCapacity(1)] [RunTimeToEmpty(2)] [Voltage(2)] [Current(2)] [PresentStatus(1)]
// 总长度: 8字节
// ============================================================================
static uint16_t build_power_summary_input(uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen < 8))
    {
        return 0U;
    }

    uint8_t idx = 0;

    // RemainingCapacity (1字节) - 百分比
    buffer[idx++] = g_ups_config.remaining_capacity;

    // RunTimeToEmpty (2字节) - 单位: 分钟
    uint16_t runtime = g_ups_config.run_time_to_empty;
    buffer[idx++] = (uint8_t)(runtime & 0xFF);
    buffer[idx++] = (uint8_t)((runtime >> 8) & 0xFF);

    // Voltage (2字节) - 单位: 100mV (0.1V)
    uint16_t voltage = g_ups_config.voltage;
    buffer[idx++] = (uint8_t)(voltage & 0xFF);
    buffer[idx++] = (uint8_t)((voltage >> 8) & 0xFF);

    // Current (2字节) - 单位: mA，有符号
    int16_t current = g_ups_config.current;
    buffer[idx++] = (uint8_t)(current & 0xFF);
    buffer[idx++] = (uint8_t)((current >> 8) & 0xFF);

    // PresentStatus (1字节)
    buffer[idx++] = pack_present_status();

    return idx;
}

// ============================================================================
// Report ID 1: Power Summary Feature Report
// 格式:
//   [iManufacturer(1)] [iProduct(1)] [iSerialNumber(1)] [iName(1)] [iDeviceChemistry(1)]
//   [CapacityMode(1)] [Rechargeable(1)]
//   [WarningCapacityLimit(1)] [RemainingCapacityLimit(1)]
//   [RemainingCapacity(1)] [FullChargeCapacity(1)] [DesignCapacity(1)]
//   [RunTimeToEmpty(2)] [RemainingTimeLimit(2)]
//   [CapacityGranularity1(1)] [CapacityGranularity2(1)]
//   [PresentStatus(1)]
// 总长度: 20字节
// ============================================================================
static uint16_t build_power_summary_feature(uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen < 20))
    {
        return 0U;
    }

    uint8_t idx = 0;

    // 字符串索引 (各1字节)
    buffer[idx++] = g_ups_config.i_manufacturer;
    buffer[idx++] = g_ups_config.i_product;
    buffer[idx++] = g_ups_config.i_serial;
    buffer[idx++] = g_ups_config.i_name;
    buffer[idx++] = g_ups_config.i_device_chemistry;

    // CapacityMode (1字节)
    buffer[idx++] = g_ups_config.capacity_mode;

    // Rechargeable (1字节)
    buffer[idx++] = g_ups_config.rechargeable ? 0x01 : 0x00;

    // 容量限制 (各1字节)
    buffer[idx++] = g_ups_config.warning_capacity_limit;
    buffer[idx++] = g_ups_config.remaining_capacity_limit;

    // 容量值 (各1字节)
    buffer[idx++] = g_ups_config.remaining_capacity;
    buffer[idx++] = g_ups_config.full_charge_capacity;
    buffer[idx++] = g_ups_config.design_capacity;

    // RunTimeToEmpty (2字节)
    buffer[idx++] = (uint8_t)(g_ups_config.run_time_to_empty & 0xFF);
    buffer[idx++] = (uint8_t)((g_ups_config.run_time_to_empty >> 8) & 0xFF);

    // RemainingTimeLimit (2字节)
    buffer[idx++] = (uint8_t)(g_ups_config.remaining_time_limit & 0xFF);
    buffer[idx++] = (uint8_t)((g_ups_config.remaining_time_limit >> 8) & 0xFF);

    // 容量粒度 (各1字节)
    buffer[idx++] = g_ups_config.capacity_granularity_1;
    buffer[idx++] = g_ups_config.capacity_granularity_2;

    // PresentStatus (1字节)
    buffer[idx++] = pack_present_status();

    return idx;
}

// ============================================================================
// 公共 API
// ============================================================================

uint16_t build_hid_input_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen)
{
    switch (report_id)
    {
        case REPORT_ID_POWER_SUMMARY:
            return build_power_summary_input(buffer, reqlen);
        default:
            return 0U;
    }
}

uint16_t build_hid_feature_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen)
{
    switch (report_id)
    {
        case REPORT_ID_POWER_SUMMARY:
            return build_power_summary_feature(buffer, reqlen);
        default:
            return 0U;
    }
}
