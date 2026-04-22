#include "ups_hid_reports.h"

#include "tusb.h"

#include "ups_data.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// APC UPS 原厂数据包格式
// Report ID 0x0C: 电量报告 (2字节: ReportID + 电量百分比)
// Report ID 0x16: 状态报告 (3字节: ReportID + 状态字节1 + 状态字节2)

// 电量报告结构 (不包含 Report ID)
typedef struct TU_ATTR_PACKED
{
    uint8_t remaining_capacity;  // 电量百分比 0-100
} apc_battery_report_t;

// 状态报告结构 (不包含 Report ID)
typedef struct TU_ATTR_PACKED
{
    uint8_t status_byte1;  // 主要状态位
    uint8_t status_byte2;  // 扩展状态位
} apc_status_report_t;

// 定义 Report ID (APC 原厂格式)
#define APC_REPORT_ID_BATTERY     0x0C  // 电量报告
#define APC_REPORT_ID_STATUS      0x16  // 状态报告

// 构建 APC 原厂格式的电量报告
// 数据包: 0C 64 (ReportID=0x0C, 电量=100%)
static uint16_t build_apc_battery_report(uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen < 1))
    {
        return 0U;
    }

    apc_battery_report_t report = {
        .remaining_capacity = g_battery.remaining_capacity,  // 电量百分比
    };

    memcpy(buffer, &report, sizeof(report));
    return (uint16_t)sizeof(report);
}

// 构建 APC 原厂格式的状态报告
// 数据包: 16 0C 00 (ReportID=0x16, 状态1=0x0C, 状态2=0x00)
// 状态字节1 (0x0C = 0000 1100):
//   bit2: 1 = 市电正常/充电中
//   bit3: 1 = 无故障/正常状态
static uint16_t build_apc_status_report(uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen < 2))
    {
        return 0U;
    }

    // 构建状态字节
    uint8_t status1 = 0x00;

    // bit0: AC Present (市电接入)
    if (g_power_summary_present_status.ac_present)
        status1 |= (1U << 0);

    // bit1: Charging (充电中)
    if (g_power_summary_present_status.charging)
        status1 |= (1U << 1);

    // bit2: Discharging (放电中) - 通常市电正常时为0
    if (g_power_summary_present_status.discharging)
        status1 |= (1U << 2);

    // bit3: Fully Charged (已充满)
    if (g_power_summary_present_status.fully_charged)
        status1 |= (1U << 3);

    // bit4: Need Replacement (需要更换电池)
    if (g_power_summary_present_status.need_replacement)
        status1 |= (1U << 4);

    // bit5: Below Remaining Capacity Limit (电量低于限制)
    if (g_power_summary_present_status.below_remaining_capacity_limit)
        status1 |= (1U << 5);

    // bit6: Battery Present (电池存在)
    if (g_power_summary_present_status.battery_present)
        status1 |= (1U << 6);

    // bit7: Overload (过载)
    if (g_power_summary_present_status.overload)
        status1 |= (1U << 7);

    // 默认状态: 市电正常、无故障 = 0x0C
    // 如果用户没有设置特定状态，使用 APC 原厂默认值
    if (status1 == 0x00 && g_power_summary_present_status.ac_present)
    {
        status1 = 0x0C;  // 市电正常、无故障
    }

    apc_status_report_t report = {
        .status_byte1 = status1,
        .status_byte2 = 0x00,  // 扩展状态，通常为0
    };

    memcpy(buffer, &report, sizeof(report));
    return (uint16_t)sizeof(report);
}

uint16_t build_hid_input_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen == 0U))
    {
        return 0U;
    }

    switch (report_id)
    {
    case APC_REPORT_ID_BATTERY:  // 0x0C - 电量报告
        return build_apc_battery_report(buffer, reqlen);

    case APC_REPORT_ID_STATUS:   // 0x16 - 状态报告
        return build_apc_status_report(buffer, reqlen);

    default:
        break;
    }

    return 0U;
}

uint16_t build_hid_feature_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen)
{
    // APC 原厂格式主要使用 Input Report
    // Feature Report 可以返回相同的数据
    return build_hid_input_report(report_id, buffer, reqlen);
}

/*
  NUT's date_conversion_reverse() for USB/HID packs a date into a 16-bit value like this:
  bits 15..9: (year - 1980) (7 bits, 0..127)
  bits 8..5: month (4 bits, 1..12)
  bits 4..0: day (5 bits, 1..31)

  E.g. For 2023/02/20, the HID numeric value is:
  year - 1980 = 43
  date = (43 << 9) + (2 << 5) + 20
  (43 << 9) = 22016 (0x5600)
  (2 << 5) = 64 (0x0040)
  + 20 (0x0014)
  Total: 22016 + 64 + 20 = 22100 decimal = 0x5654
*/
int pack_hid_date_mmddyy(const char *s, uint16_t *out)
{
    unsigned int mm, dd, yy;

    if (!s || !out)
        return 0;

    /* Accept exactly "MM/DD/YY" with 2-digit fields.
       (If you want to accept "8/6/23", adjust the sscanf format.) */
    if (sscanf(s, "%2u/%2u/%2u", &mm, &dd, &yy) != 3)
        return 0;

    if (mm < 1 || mm > 12 || dd < 1 || dd > 31)
        return 0;

    /* Interpret YY as 2000..2099 by default (common expectation for 2-digit years).
       If you want a sliding window, change this mapping. */
    unsigned int year = 2000u + yy;

    if (year < 1980u || year > 2107u)
        return 0;

    unsigned int yoff = year - 1980u; /* 0..127 */

    uint16_t packed = (uint16_t)((yoff << 9) | (mm << 5) | dd);
    *out = packed;
    return 1;
}
