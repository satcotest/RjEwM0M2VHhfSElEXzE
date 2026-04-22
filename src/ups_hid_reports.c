#include "ups_hid_reports.h"
#include "ups_hid_config.h"
#include <string.h>

// 全局配置
extern ups_hid_config_t g_ups_config;

// 报告大小定义
#define INPUT_REPORT_SIZE       8
#define FEATURE_REPORT_SIZE     20

// 打包 PresentStatus 位字段 (5位状态 + 3位填充 = 8位)
static uint8_t pack_present_status(void)
{
    uint8_t bits = 0;
    
    bits |= g_ups_config.status1.bits.ac_present      ? (1 << 0) : 0;
    bits |= g_ups_config.status1.bits.charging        ? (1 << 1) : 0;
    bits |= g_ups_config.status1.bits.discharging     ? (1 << 2) : 0;
    bits |= g_ups_config.status1.bits.fully_charged   ? (1 << 3) : 0;
    bits |= g_ups_config.status1.bits.battery_present ? (1 << 4) : 0;
    
    return bits;
}

// 写入8位值到缓冲区
static inline void write_u8(uint8_t **buf, uint8_t val)
{
    **buf = val;
    (*buf)++;
}

// 写入16位值到缓冲区 (小端序)
static inline void write_u16(uint8_t **buf, uint16_t val)
{
    write_u8(buf, (uint8_t)(val & 0xFF));
    write_u8(buf, (uint8_t)((val >> 8) & 0xFF));
}

// 写入16位有符号值到缓冲区 (小端序)
static inline void write_s16(uint8_t **buf, int16_t val)
{
    write_u16(buf, (uint16_t)val);
}

// ============================================================================
// Report ID 1: Power Summary Input Report (8字节)
// [RemainingCapacity(1)] [RunTimeToEmpty(2)] [Voltage(2)] [Current(2)] [PresentStatus(1)]
// ============================================================================
static uint16_t build_power_summary_input(uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen < INPUT_REPORT_SIZE))
    {
        return 0;
    }

    uint8_t *p = buffer;
    
    write_u8(&p, g_ups_config.remaining_capacity);
    write_u16(&p, g_ups_config.run_time_to_empty);
    write_u16(&p, g_ups_config.voltage);
    write_s16(&p, g_ups_config.current);
    write_u8(&p, pack_present_status());

    return INPUT_REPORT_SIZE;
}

// ============================================================================
// Report ID 1: Power Summary Feature Report (20字节)
// ============================================================================
static uint16_t build_power_summary_feature(uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen < FEATURE_REPORT_SIZE))
    {
        return 0;
    }

    uint8_t *p = buffer;
    
    // 字符串索引 (5字节)
    write_u8(&p, g_ups_config.i_manufacturer);
    write_u8(&p, g_ups_config.i_product);
    write_u8(&p, g_ups_config.i_serial);
    write_u8(&p, g_ups_config.i_name);
    write_u8(&p, g_ups_config.i_device_chemistry);
    
    // 模式和属性 (3字节)
    write_u8(&p, g_ups_config.capacity_mode);
    write_u8(&p, g_ups_config.rechargeable ? 1 : 0);
    
    // 容量限制 (2字节)
    write_u8(&p, g_ups_config.warning_capacity_limit);
    write_u8(&p, g_ups_config.remaining_capacity_limit);
    
    // 容量值 (3字节)
    write_u8(&p, g_ups_config.remaining_capacity);
    write_u8(&p, g_ups_config.full_charge_capacity);
    write_u8(&p, g_ups_config.design_capacity);
    
    // 时间值 (4字节)
    write_u16(&p, g_ups_config.run_time_to_empty);
    write_u16(&p, g_ups_config.remaining_time_limit);
    
    // 容量粒度 (2字节)
    write_u8(&p, g_ups_config.capacity_granularity_1);
    write_u8(&p, g_ups_config.capacity_granularity_2);
    
    // 状态 (1字节)
    write_u8(&p, pack_present_status());

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
    return 0;
}

uint16_t build_hid_feature_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen)
{
    if (report_id == REPORT_ID_POWER_SUMMARY)
    {
        return build_power_summary_feature(buffer, reqlen);
    }
    return 0;
}
