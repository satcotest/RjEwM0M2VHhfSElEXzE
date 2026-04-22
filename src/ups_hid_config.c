#include "ups_hid_config.h"

// 全局配置 (在 main.c 中定义)
extern ups_hid_config_t g_ups_config;

void ups_hid_config_init(void)
{
    g_ups_config = (ups_hid_config_t)UPS_HID_DEFAULT_CONFIG();
}

void ups_hid_set_battery_level(uint8_t level)
{
    if (level > 100)
    {
        level = 100;
    }
    g_ups_config.remaining_capacity = level;
}

void ups_hid_set_status(bool ac_present, bool charging, bool battery_present)
{
    g_ups_config.status1.bits.ac_present = ac_present;
    g_ups_config.status1.bits.charging = charging;
    g_ups_config.status1.bits.battery_present = battery_present;

    // 自动设置其他相关状态
    if (charging)
    {
        g_ups_config.status1.bits.discharging = false;
        g_ups_config.status2.bits.fully_charged = (g_ups_config.remaining_capacity >= 100);
    }
    else if (!ac_present)
    {
        g_ups_config.status1.bits.discharging = true;
        g_ups_config.status2.bits.fully_charged = false;
    }
}

uint8_t ups_hid_get_battery_level(void)
{
    return g_ups_config.remaining_capacity;
}

uint8_t ups_hid_get_status_byte1(void)
{
    return g_ups_config.status1.value;
}
