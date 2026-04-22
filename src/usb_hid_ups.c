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

// 前向声明
static uint16_t handle_feature_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen);

void ups_hid_periodic_task(void)
{
    uint32_t const now_ms = HAL_GetTick();
    if ((now_ms - hid_last_report_ms) < g_ups_config.report_interval_ms)
    {
        return;
    }

    hid_last_report_ms = now_ms;

    if (!tud_hid_ready())
    {
        return;
    }

    // 轮流发送电量报告和状态报告
    uint8_t buffer[2];
    uint16_t len;

    if (hid_report_cycle_index == 0)
    {
        // 发送电量报告 (Report ID 0x0C)
        len = build_hid_input_report(APC_REPORT_ID_BATTERY, buffer, sizeof(buffer));
        if (len > 0)
        {
            if (tud_hid_report(APC_REPORT_ID_BATTERY, buffer, len))
            {
                hid_report_cycle_index = 1;
            }
            // 如果发送失败，下次重试
        }
        else
        {
            hid_report_cycle_index = 1;
        }
    }
    else
    {
        // 发送状态报告 (Report ID 0x16)
        len = build_hid_input_report(APC_REPORT_ID_STATUS, buffer, sizeof(buffer));
        if (len > 0)
        {
            if (tud_hid_report(APC_REPORT_ID_STATUS, buffer, len))
            {
                hid_report_cycle_index = 0;
            }
            // 如果发送失败，下次重试
        }
        else
        {
            hid_report_cycle_index = 0;
        }
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
        return handle_feature_report(report_id, buffer, reqlen);
    }

    return 0U;
}

// 处理 Feature Report 请求 (hidups.sys 握手流程)
static uint16_t handle_feature_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen)
{
    if ((buffer == NULL) || (reqlen == 0U))
    {
        return 0U;
    }

    switch (report_id)
    {
    case APC_FEATURE_ID_PRODUCT:
        // iProduct: 返回产品字符串索引
        if (reqlen >= 1)
        {
            buffer[0] = g_ups_config.i_product;
            return 1;
        }
        break;

    case APC_FEATURE_ID_SERIAL:
        // iSerialNumber: 返回序列号字符串索引
        if (reqlen >= 1)
        {
            buffer[0] = g_ups_config.i_serial;
            return 1;
        }
        break;

    case APC_FEATURE_ID_RECHARGEABLE:
        // Rechargeable: 返回 1 (可充电)
        if (reqlen >= 1)
        {
            buffer[0] = 0x01;
            return 1;
        }
        break;

    case APC_FEATURE_ID_CAPACITY_MODE:
        // CapacityMode: 返回百分比模式 (0x02)
        if (reqlen >= 1)
        {
            buffer[0] = g_ups_config.capacity_mode;
            return 1;
        }
        break;

    case APC_FEATURE_ID_FULL_CHARGE:
        // FullChargeCapacity: 返回满电容量
        if (reqlen >= 1)
        {
            buffer[0] = g_ups_config.full_charge_capacity;
            return 1;
        }
        break;

    case APC_FEATURE_ID_DESIGN_CAPACITY:
        // DesignCapacity: 返回设计容量
        if (reqlen >= 1)
        {
            buffer[0] = g_ups_config.design_capacity;
            return 1;
        }
        break;

    case APC_REPORT_ID_BATTERY:
        // 电量报告 (Feature 版本)
        return build_hid_feature_report(report_id, buffer, reqlen);

    case APC_FEATURE_ID_CHEMISTRY:
        // iDeviceChemistry: 返回字符串索引
        if (reqlen >= 1)
        {
            buffer[0] = 0x04;  // PbAc 字符串索引
            return 1;
        }
        break;

    case APC_FEATURE_ID_OEM:
        // iOEMInformation: 返回字符串索引
        if (reqlen >= 1)
        {
            buffer[0] = 0x05;  // APC 字符串索引
            return 1;
        }
        break;

    default:
        // 对于未知的 Report ID，返回 0 长度数据而不是 STALL
        // 这可以避免 Windows 重置管道
        break;
    }

    return 0U;
}
