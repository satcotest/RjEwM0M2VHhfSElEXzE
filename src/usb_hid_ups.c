#include "ups_hid_device.h"
#include "ups_hid_reports.h"
#include "ups_hid_config.h"

#include "stm32f1xx_hal.h"
#include "tusb.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// 全局配置
extern ups_hid_config_t g_ups_config;

static uint32_t hid_last_report_ms;

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

    uint8_t buffer[8];
    uint16_t const len = build_hid_input_report(REPORT_ID_POWER_SUMMARY, buffer, sizeof(buffer));
    if (len > 0)
    {
        (void)tud_hid_report(REPORT_ID_POWER_SUMMARY, buffer, len);
        hid_last_report_ms = now_ms;
    }
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
