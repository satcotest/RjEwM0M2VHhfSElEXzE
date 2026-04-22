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

    // 自动更新充满状态
    if (level >= 100)
    {
        g_ups_config.status1.bits.fully_charged = true;
    }
    else
    {
        g_ups_config.status1.bits.fully_charged = false;
    }
}

void ups_hid_set_status(bool ac_present, bool charging, bool battery_present)
{
    g_ups_config.status1.bits.ac_present = ac_present;
    g_ups_config.status1.bits.charging = charging;
    g_ups_config.status1.bits.battery_present = battery_present;

    // 自动设置放电状态
    if (ac_present && charging)
    {
        g_ups_config.status1.bits.discharging = false;
        g_ups_config.current = (g_ups_config.current > 0) ? g_ups_config.current : 500; // 默认充电电流 500mA
    }
    else if (!ac_present)
    {
        g_ups_config.status1.bits.discharging = true;
        g_ups_config.status1.bits.charging = false;
        g_ups_config.current = (g_ups_config.current < 0) ? g_ups_config.current : -500; // 默认放电电流 -500mA
    }
    else
    {
        g_ups_config.status1.bits.discharging = false;
        g_ups_config.current = 0;
    }
}

void ups_hid_set_voltage(uint16_t voltage_100mv)
{
    g_ups_config.voltage = voltage_100mv;
    g_ups_config.battery_voltage = voltage_100mv;
}

void ups_hid_set_current(int16_t current_ma)
{
    g_ups_config.current = current_ma;
    g_ups_config.battery_current = current_ma;
}

void ups_hid_set_temperature(uint16_t temp_01c)
{
    g_ups_config.temperature = temp_01c;
}

uint8_t ups_hid_get_battery_level(void)
{
    return g_ups_config.remaining_capacity;
}

uint8_t ups_hid_get_status_byte1(void)
{
    return g_ups_config.status1.value;
}
