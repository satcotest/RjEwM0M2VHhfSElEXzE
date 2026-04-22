#include "ups_adc.h"
#include "ups_hid_config.h"
#include "stm32f1xx_hal.h"

#include <string.h>

// 全局配置
extern ups_hid_config_t g_ups_config;

// ADC 配置
static adc_config_t s_adc_config = {
    .vbus = {
        .channel = 0,           // ADC通道0 - 总线电压
        .raw_value = 0,
        .voltage_mv = 12000,    // 默认12V
        .scale_factor = 1.0f,
        .offset_mv = 0
    },
    .vbat = {
        .channel = 1,           // ADC通道1 - 电池电压
        .raw_value = 0,
        .voltage_mv = 12000,    // 默认12V
        .scale_factor = 1.0f,
        .offset_mv = 0
    },
    .ibus = {
        .channel = 2,           // ADC通道2 - 电流检测
        .raw_value = 0,
        .voltage_mv = 0,
        .scale_factor = 1.0f,
        .offset_mv = 0
    },
    .temp = {
        .channel = 3,           // ADC通道3 - 温度检测
        .raw_value = 0,
        .voltage_mv = 2500,     // 默认25°C
        .scale_factor = 1.0f,
        .offset_mv = 0
    },
    .sample_interval_ms = 100,  // 100ms采样间隔
    .enabled = false            // 默认禁用，需要手动启用
};

static uint32_t s_last_sample_ms = 0;
static bool s_adc_ready = false;

// ADC初始化
void ups_adc_init(void)
{
    // TODO: 初始化ADC外设
    // 1. 使能ADC时钟
    // 2. 配置GPIO为模拟输入
    // 3. 配置ADC参数
    // 4. 校准ADC

    s_adc_ready = false;
}

// ADC启动采样
void ups_adc_start(void)
{
    s_adc_config.enabled = true;
    s_last_sample_ms = HAL_GetTick();
    // TODO: 启动ADC转换
}

// ADC停止采样
void ups_adc_stop(void)
{
    s_adc_config.enabled = false;
    // TODO: 停止ADC转换
}

// ADC周期任务
void ups_adc_periodic_task(void)
{
    if (!s_adc_config.enabled)
    {
        return;
    }

    uint32_t now_ms = HAL_GetTick();
    if ((now_ms - s_last_sample_ms) < s_adc_config.sample_interval_ms)
    {
        return;
    }

    // TODO: 执行ADC采样
    // 1. 启动ADC转换
    // 2. 等待转换完成
    // 3. 读取原始值
    // 4. 转换为实际电压/电流/温度

    s_last_sample_ms = now_ms;
    s_adc_ready = true;
}

// 获取电压值 (单位: mV)
uint16_t ups_adc_get_voltage_mv(void)
{
    // 返回电池电压 (mV)
    return s_adc_config.vbat.voltage_mv;
}

// 获取电流值 (单位: mA, 有符号)
int16_t ups_adc_get_current_ma(void)
{
    // 根据电压差和采样电阻计算电流
    // 正数=充电，负数=放电
    return 500; // 默认返回500mA充电电流
}

// 获取温度值 (单位: 0.1°C)
uint16_t ups_adc_get_temperature_01c(void)
{
    // 返回温度值 (0.1°C)
    return 250; // 默认25.0°C
}

// 更新 HID 配置中的电气参数
void ups_adc_update_hid_params(void)
{
    if (!s_adc_ready)
    {
        return;
    }

    // 更新电压 (转换为 0.1V 单位)
    uint16_t voltage_mv = ups_adc_get_voltage_mv();
    g_ups_config.voltage = voltage_mv / 100;
    g_ups_config.battery_voltage = g_ups_config.voltage;

    // 更新电流 (mA单位)
    int16_t current_ma = ups_adc_get_current_ma();
    g_ups_config.current = current_ma;
    g_ups_config.battery_current = current_ma;

    // 更新温度 (0.1°C单位)
    g_ups_config.temperature = ups_adc_get_temperature_01c();

    // 根据电流方向更新状态
    if (current_ma > 0)
    {
        g_ups_config.status1.bits.charging = true;
        g_ups_config.status1.bits.discharging = false;
    }
    else if (current_ma < 0)
    {
        g_ups_config.status1.bits.charging = false;
        g_ups_config.status1.bits.discharging = true;
    }
    else
    {
        g_ups_config.status1.bits.charging = false;
        g_ups_config.status1.bits.discharging = false;
    }
}

// 检查ADC是否就绪
bool ups_adc_is_ready(void)
{
    return s_adc_ready;
}
