#ifndef UPS_HID_CONFIG_H
#define UPS_HID_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

// Report IDs (通用 HID Power Device)
#define REPORT_ID_POWER_SUMMARY  0x01  // Power Summary (Input + Feature)
#define REPORT_ID_BATTERY        0x02  // Battery System (Feature)

// Capacity Mode 定义
#define CAPACITY_MODE_MAH     0  // 毫安时
#define CAPACITY_MODE_MWH     1  // 毫瓦时
#define CAPACITY_MODE_PERCENT 2  // 百分比

// PresentStatus 位定义 (与 HID Descriptor 顺序一致)
// bit 0: AC Present
// bit 1: Charging
// bit 2: Discharging
// bit 3: Fully Charged
// bit 4: Need Replacement
// bit 5: Below Remaining Capacity Limit
// bit 6: Battery Present
// bit 7: Overload
// bit 8: Shutdown Imminent

// 状态字节1
typedef union {
    struct {
        uint8_t ac_present                    : 1;  // bit 0: 市电正常
        uint8_t charging                      : 1;  // bit 1: 充电中
        uint8_t discharging                   : 1;  // bit 2: 放电中
        uint8_t fully_charged                 : 1;  // bit 3: 已充满
        uint8_t need_replacement              : 1;  // bit 4: 需要更换电池
        uint8_t below_remaining_capacity_limit: 1;  // bit 5: 电量低于限制
        uint8_t battery_present               : 1;  // bit 6: 电池存在
        uint8_t overload                      : 1;  // bit 7: 过载
    } bits;
    uint8_t value;
} ups_status_byte1_t;

// 状态字节2
typedef union {
    struct {
        uint8_t shutdown_imminent             : 1;  // bit 0: 即将关机
        uint8_t reserved1                     : 7;  // bit 1-7: 保留
    } bits;
    uint8_t value;
} ups_status_byte2_t;

// UPS 配置结构体 (通用 HID Power Device)
typedef struct {
    // === 容量设置 ===
    uint8_t remaining_capacity;        // 当前电量 0-100
    uint8_t full_charge_capacity;      // 满电容量 0-100
    uint8_t design_capacity;           // 设计容量 0-100
    uint8_t warning_capacity_limit;    // 警告电量限制 0-100
    uint8_t remaining_capacity_limit;  // 剩余电量限制 0-100
    uint8_t capacity_mode;             // 容量模式: 0=mAh, 1=mWh, 2=百分比
    uint8_t capacity_granularity_1;    // 容量粒度1
    uint8_t capacity_granularity_2;    // 容量粒度2

    // === 时间设置 ===
    uint16_t run_time_to_empty;        // 剩余运行时间 (分钟)
    uint16_t remaining_time_limit;     // 剩余时间限制 (分钟)

    // === 电气参数 ===
    uint16_t voltage;                  // 电压 (单位: 100mV = 0.1V)
    int16_t  current;                  // 电流 (单位: mA, 正=充电, 负=放电)
    uint16_t battery_voltage;          // 电池电压 (单位: 100mV)
    int16_t  battery_current;          // 电池电流 (单位: mA)
    uint16_t temperature;              // 温度 (单位: 0.1°C)

    // === 状态设置 ===
    ups_status_byte1_t status1;
    ups_status_byte2_t status2;

    // === 设备属性 ===
    bool rechargeable;                 // 是否可充电
    uint8_t i_manufacturer;            // 制造商字符串索引
    uint8_t i_product;                 // 产品字符串索引
    uint8_t i_serial;                  // 序列号字符串索引
    uint8_t i_name;                    // 名称字符串索引
    uint8_t i_device_chemistry;        // 电池化学类型字符串索引

    // === 报告间隔 ===
    uint32_t report_interval_ms;       // 报告间隔 (毫秒)
} ups_hid_config_t;

// ADC 通道配置 (为以后ADC功能预留)
typedef struct {
    uint8_t  channel;           // ADC通道号
    uint16_t raw_value;         // 原始ADC值
    uint16_t voltage_mv;        // 转换后的电压值(mV)
    float    scale_factor;      // 缩放系数
    uint16_t offset_mv;         // 偏移量(mV)
} adc_channel_config_t;

// ADC 配置结构体
typedef struct {
    adc_channel_config_t vbus;      // 总线电压检测
    adc_channel_config_t vbat;      // 电池电压检测
    adc_channel_config_t ibus;      // 总线电流检测
    adc_channel_config_t temp;      // 温度检测
    uint32_t sample_interval_ms;    // 采样间隔
    bool     enabled;               // ADC是否启用
} adc_config_t;

// 默认配置 (通用 UPS)
#define UPS_HID_DEFAULT_CONFIG() { \
    .remaining_capacity = 100, \
    .full_charge_capacity = 100, \
    .design_capacity = 100, \
    .warning_capacity_limit = 20, \
    .remaining_capacity_limit = 10, \
    .capacity_mode = CAPACITY_MODE_PERCENT, \
    .capacity_granularity_1 = 1, \
    .capacity_granularity_2 = 1, \
    .run_time_to_empty = 600, \
    .remaining_time_limit = 120, \
    .voltage = 120, \
    .current = 500, \
    .battery_voltage = 120, \
    .battery_current = 500, \
    .temperature = 250, \
    .status1 = { .value = 0x4F }, \
    .status2 = { .value = 0x00 }, \
    .rechargeable = true, \
    .i_manufacturer = 1, \
    .i_product = 2, \
    .i_serial = 3, \
    .i_name = 2, \
    .i_device_chemistry = 1, \
    .report_interval_ms = 1000, \
}

// 函数声明
void ups_hid_config_init(void);
void ups_hid_set_battery_level(uint8_t level);
void ups_hid_set_status(bool ac_present, bool charging, bool battery_present);
void ups_hid_set_voltage(uint16_t voltage_100mv);
void ups_hid_set_current(int16_t current_ma);
void ups_hid_set_temperature(uint16_t temp_01c);
uint8_t ups_hid_get_battery_level(void);
uint8_t ups_hid_get_status_byte1(void);

#endif // UPS_HID_CONFIG_H
