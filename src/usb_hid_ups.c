#include "ups_hid_device.h"

#include "ups_hid_reports.h"

#include "stm32f1xx_hal.h"
#include "tusb.h"

#include <stdbool.h>
#include <stdint.h>

// APC 原厂 Report ID
#define APC_REPORT_ID_BATTERY     0x0C  // 电量报告
#define APC_REPORT_ID_STATUS      0x16  // 状态报告

static uint32_t hid_last_report_ms;
static uint8_t hid_report_cycle_index;

static void reset_hid_timing_state(void)
{
    hid_last_report_ms = 0U;
    hid_report_cycle_index = 0U;
}

void ups_hid_periodic_task(void)
{
    // Send Input Report more frequently for Windows battery display
    uint32_t const now_ms = HAL_GetTick();
    if ((now_ms - hid_last_report_ms) < 1000U)  // 每秒发送一次
    {
        return;
    }

    hid_last_report_ms = now_ms;

    if (!tud_hid_ready())
    {
        return;
    }

    // 轮流发送电量报告和状态报告
    // 注意: TinyUSB 的 tud_hid_report 第一个参数是 report_id
    // 数据包格式: ReportID + Data
    if (hid_report_cycle_index == 0)
    {
        // 发送电量报告 (Report ID 0x0C)
        // 数据包格式: 0C 64 (ReportID=0x0C, 电量=100%)
        uint8_t battery_report[1] = {100};  // 电量 100%
        (void)tud_hid_report(APC_REPORT_ID_BATTERY, battery_report, sizeof(battery_report));
        hid_report_cycle_index = 1;
    }
    else
    {
        // 发送状态报告 (Report ID 0x16)
        // 数据包格式: 16 0C 00 (ReportID=0x16, 状态1=0x0C, 状态2=0x00)
        uint8_t status_report[2] = {0x0C, 0x00};  // 状态字节
        (void)tud_hid_report(APC_REPORT_ID_STATUS, status_report, sizeof(status_report));
        hid_report_cycle_index = 0;
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
        // 根据 Report ID 返回对应的数据
        if (report_id == APC_REPORT_ID_BATTERY)
        {
            // 电量报告: 返回 1 字节电量
            if (reqlen >= 1)
            {
                buffer[0] = 100;  // 电量 100%
                return 1;
            }
        }
        else if (report_id == APC_REPORT_ID_STATUS)
        {
            // 状态报告: 返回 2 字节状态
            if (reqlen >= 2)
            {
                buffer[0] = 0x0C;  // 状态1: 市电正常
                buffer[1] = 0x00;  // 状态2
                return 2;
            }
        }
    }

    return 0U;
}

// Mount and unmount callbacks to prevent usb failures due to stale state.
void tud_mount_cb(void)
{
    reset_hid_timing_state();
}

void tud_umount_cb(void)
{
    reset_hid_timing_state();
}

void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;
}

void tud_resume_cb(void)
{
    reset_hid_timing_state();
}
