#include "ups_hid_reports.h"
#include "ups_hid_config.h"

#include <stdio.h>
#include <string.h>

// 全局配置 (在 main.c 中定义)
extern ups_hid_config_t g_ups_config;

// 构建 APC 原厂格式的电量报告 (Report ID 0x0C)
// 数据包: 0C 64 (ReportID=0x0C, 电量=100%)
static uint16_t build_battery_report(uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen < 1))
    {
        return 0U;
    }

    buffer[0] = ups_hid_get_battery_level();
    return 1U;
}

// 构建 APC 原厂格式的状态报告 (Report ID 0x16)
// 数据包: 16 0C 00 (ReportID=0x16, 状态1=0x0C, 状态2=0x00)
static uint16_t build_status_report(uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen < 1))
    {
        return 0U;
    }

    buffer[0] = ups_hid_get_status_byte1();
    return 1U;
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
        return build_battery_report(buffer, reqlen);

    case APC_REPORT_ID_STATUS:   // 0x16 - 状态报告
        return build_status_report(buffer, reqlen);

    default:
        break;
    }

    return 0U;
}

uint16_t build_hid_feature_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen == 0U))
    {
        return 0U;
    }

    uint8_t value = 0xFF;

    switch (report_id)
    {
    case APC_FEATURE_ID_PRODUCT:
        value = g_ups_config.i_product;
        break;

    case APC_FEATURE_ID_SERIAL:
        value = g_ups_config.i_serial;
        break;

    case APC_FEATURE_ID_CHEMISTRY:
        value = 0x04;  // PbAc
        break;

    case APC_FEATURE_ID_OEM:
        value = 0x05;  // APC
        break;

    case APC_FEATURE_ID_RECHARGEABLE:
        value = 0x01;  // Yes
        break;

    case APC_FEATURE_ID_CAPACITY_MODE:
        value = g_ups_config.capacity_mode;
        break;

    case APC_FEATURE_ID_FULL_CHARGE:
        value = g_ups_config.full_charge_capacity;
        break;

    case APC_FEATURE_ID_DESIGN_CAPACITY:
        value = g_ups_config.design_capacity;
        break;

    case APC_REPORT_ID_BATTERY:
        return build_battery_report(buffer, reqlen);

    case APC_REPORT_ID_STATUS:
        return build_status_report(buffer, reqlen);

    default:
        return 0U;
    }

    if (reqlen >= 1)
    {
        buffer[0] = value;
        return 1;
    }

    return 0U;
}

/*
  NUT's date_conversion_reverse() for USB/HID packs a date into a 16-bit value:
  bits 15..9: (year - 1980) (7 bits, 0..127)
  bits 8..5: month (4 bits, 1..12)
  bits 4..0: day (5 bits, 1..31)

  E.g. For 2023/02/20:
  year - 1980 = 43
  date = (43 << 9) + (2 << 5) + 20 = 0x5654
*/
int pack_hid_date_mmddyy(const char *s, uint16_t *out)
{
    unsigned int mm, dd, yy;

    if (!s || !out)
        return 0;

    if (sscanf(s, "%2u/%2u/%2u", &mm, &dd, &yy) != 3)
        return 0;

    if (mm < 1 || mm > 12 || dd < 1 || dd > 31)
        return 0;

    unsigned int year = 2000u + yy;
    if (year < 1980u || year > 2107u)
        return 0;

    unsigned int yoff = year - 1980u;
    uint16_t packed = (uint16_t)((yoff << 9) | (mm << 5) | dd);
    *out = packed;
    return 1;
}
