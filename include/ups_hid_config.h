#ifndef UPS_HID_CONFIG_H_
#define UPS_HID_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// APC UPS Report IDs (原厂格式)
#define APC_REPORT_ID_BATTERY     0x0C  // 电量报告
#define APC_REPORT_ID_STATUS      0x16  // 状态报告

// Feature Report IDs (hidups.sys 握手流程)
#define APC_FEATURE_ID_PRODUCT          0x01  // iProduct
#define APC_FEATURE_ID_SERIAL           0x02  // iSerialNumber
#define APC_FEATURE_ID_CHEMISTRY        0x03  // iDeviceChemistry
#define APC_FEATURE_ID_OEM              0x04  // iOEMInformation
#define APC_FEATURE_ID_RECHARGEABLE     0x05  // Rechargeable
#define APC_FEATURE_ID_CAPACITY_MODE    0x06  // CapacityMode
#define APC_FEATURE_ID_FULL_CHARGE      0x07  // FullChargeCapacity
#define APC_FEATURE_ID_DESIGN_CAPACITY  0x08  // DesignCapacity

// 状态字节位定义 (APC 原厂格式)
// 状态字节1 (对应 APC 抓包中的 0x0C)
typedef union {
    struct {
        uint8_t charging                      : 1;  // bit 0: 充电中
        uint8_t discharging                   : 1;  // bit 1: 放电中
        uint8_t ac_present                    : 1;  // bit 2: 市电正常
        uint8_t battery_present               : 1;  // bit 3: 电池存在
        uint8_t below_remaining_capacity_limit: 1;  // bit 4: 电量低于限制
        uint8_t shutdown_imminent             : 1;  // bit 5: 即将关机
        uint8_t remaining_time_limit_expired  : 1;  // bit 6: 剩余时间限制到期
        uint8_t need_replacement              : 1;  // bit 7: 需要更换电池
    } bits;
    uint8_t value;
} apc_status_byte1_t;

// 状态字节2 (扩展状态)
typedef union {
    struct {
        uint8_t fully_charged   : 1;  // bit 0: 已充满
        uint8_t overload        : 1;  // bit 1: 过载
        uint8_t reserved        : 6;  // bit 2-7: 保留
    } bits;
    uint8_t value;
} apc_status_byte2_t;

// UPS 配置结构体 (便于修改参数)
typedef struct {
    // 电量设置
    uint8_t remaining_capacity;      // 当前电量 0-100
    uint8_t full_charge_capacity;    // 满电容量 0-100
    uint8_t design_capacity;         // 设计容量 0-100
    uint8_t capacity_mode;           // 容量模式: 0=mAh, 1=mWh, 2=百分比

    // 状态设置
    apc_status_byte1_t status1;
    apc_status_byte2_t status2;

    // 字符串索引
    uint8_t i_product;
    uint8_t i_serial;

    // 报告间隔 (毫秒)
    uint32_t report_interval_ms;
} ups_hid_config_t;

// 默认配置
#define UPS_HID_DEFAULT_CONFIG() { \
    .remaining_capacity = 100, \
    .full_charge_capacity = 100, \
    .design_capacity = 100, \
    .capacity_mode = 2, \
    .status1 = { .value = 0x0D }, \
    .status2 = { .value = 0x00 }, \
    .i_product = 2, \
    .i_serial = 3, \
    .report_interval_ms = 1000, \
}

// 默认状态: 市电正常 + 充电中 + 电池存在
// 0x0D = 00001101b = Charging(1) + AC正常(4) + 电池存在(8)
#define APC_STATUS_DEFAULT 0x0D

// 初始化配置为默认值
void ups_hid_config_init(void);

// 设置电量百分比 (0-100)
void ups_hid_set_battery_level(uint8_t level);

// 设置状态
void ups_hid_set_status(bool ac_present, bool charging, bool battery_present);

// 获取当前电量
uint8_t ups_hid_get_battery_level(void);

// 获取当前状态字节
uint8_t ups_hid_get_status_byte1(void);

#ifdef __cplusplus
}
#endif

#endif // UPS_HID_CONFIG_H_
