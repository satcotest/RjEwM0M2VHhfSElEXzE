#include "ups_hid_device.h"
#include "ups_hid_reports.h"
#include "ups_hid_config.h"

#include "stm32f1xx_hal.h"
#include "tusb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// 全局配置 (在 main.c 中定义)
extern ups_hid_config_t g_ups_config;

static uint32_t hid_last_report_ms;
static uint8_t hid_report_cycle_index;

void ups_hid_periodic_task(void)
{
    uint32_t const now_ms = HAL_GetTick();
    if ((now_ms - hid_last_report_ms) < g_ups_config.report_interval_ms)
    {
        return;
    }

    if (!tud_hid_ready())
    {
        return;
    }

    uint8_t buffer[1];
    uint16_t len;

    // 轮流发送电量报告和状态报告
    if (hid_report_cycle_index == 0)
    {
        // 发送电量报告 (Report ID 0x0C) - 1字节
        len = build_hid_input_report(APC_REPORT_ID_BATTERY, buffer, sizeof(buffer));
        if (len > 0)
        {
            (void)tud_hid_report(APC_REPORT_ID_BATTERY, buffer, len);
        }
    }
    else
    {
        // 发送状态报告 (Report ID 0x16) - 1字节
        len = build_hid_input_report(APC_REPORT_ID_STATUS, buffer, sizeof(buffer));
        if (len > 0)
        {
            (void)tud_hid_report(APC_REPORT_ID_STATUS, buffer, len);
        }
    }

    // 无论发送成功与否，都更新时间和切换周期
    hid_last_report_ms = now_ms;
    hid_report_cycle_index ^= 1;
}

void tud_hid_set_report_cb(uint8_t instance,
                           uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const *buffer,
                           uint16_t bufsize)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}

uint16_t tud_hid_get_report_cb(uint8_t instance,
                               uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t *buffer,
                               uint16_t reqlen)
{
    (void)instance;

    if (report_type == HID_REPORT_TYPE_INPUT)
    {
        return build_hid_input_report(report_id, buffer, reqlen);
    }
    else if (report_type == HID_REPORT_TYPE_FEATURE)
    {
        return build_hid_feature_report(report_id, buffer, reqlen);
    }

    return 0U;
}
