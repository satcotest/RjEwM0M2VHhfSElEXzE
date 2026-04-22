#include "tusb.h"
#include "usb_descriptors.h"

/*
 * 通用 HID Power Device (UPS) 实现
 * 支持 Windows/Linux/群晖/威联通
 * 参考 STM32_UPS_HID_PowerDevice 和 HIDPowerDevice 项目
 */

//--------------------------------------------------------------------+
// HID Descriptor 辅助宏 (使用 UPS_ 前缀避免与 TinyUSB 冲突)
//--------------------------------------------------------------------+

// Usage Page 宏
#define UPS_USAGE_PAGE(x)       0x05, x
#define UPS_USAGE_PAGE16(x)     0x06, (x & 0xFF), (x >> 8)

// Usage 宏
#define UPS_USAGE(x)            0x09, x
#define UPS_USAGE16(x)          0x0A, (x & 0xFF), (x >> 8)

// Collection 宏
#define UPS_COLLECTION(x)       0xA1, x
#define UPS_END_COLLECTION      0xC0

// Report ID
#define UPS_REPORT_ID(x)        0x85, x

// Input/Feature 宏
#define UPS_INPUT(x)            0x81, x
#define UPS_FEATURE(x)          0xB1, x

// 报告大小和数量
#define UPS_REPORT_SIZE(x)      0x75, x
#define UPS_REPORT_COUNT(x)     0x95, x

// 逻辑最小/最大值
#define UPS_LOGICAL_MIN8(x)     0x15, (int8_t)(x)
#define UPS_LOGICAL_MAX8(x)     0x25, (int8_t)(x)
#define UPS_LOGICAL_MIN16(x)    0x16, (x & 0xFF), ((x >> 8) & 0xFF)
#define UPS_LOGICAL_MAX16(x)    0x26, (x & 0xFF), ((x >> 8) & 0xFF)

// 常用报告属性
#define UPS_DATA_VAR_ABS        0x02
#define UPS_DATA_VAR_ABS_VOL    0x82
#define UPS_CONST_VAR_ABS       0x01

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    // APC UPS 原厂 VID/PID (可修改为通用UPS)
    .idVendor = 0x051D,
    .idProduct = 0x0002,
    .bcdDevice = 0x0106,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01
};

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
// 优化版本 - 使用宏定义提高可读性
// Report ID 1: Power Summary (Input + Feature)
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] =
{
    // === Power Device (UPS) Application Collection ===
    UPS_USAGE_PAGE(0x84),           // Power Device
    UPS_USAGE(0x04),                // UPS
    UPS_COLLECTION(0x01),           // Application

    // =====================================================
    // Report ID 1: Power Summary
    // =====================================================
    UPS_USAGE(0x24),                // Power Summary
    UPS_COLLECTION(0x02),           // Logical
    UPS_REPORT_ID(0x01),

    // === INPUT 报告 (8字节) ===
    // [RemainingCapacity(1)] [RunTimeToEmpty(2)] [Voltage(2)] [Current(2)] [PresentStatus(1)]

    // RemainingCapacity - 8位百分比
    UPS_USAGE_PAGE(0x85),           // Battery System
    UPS_USAGE(0x66),                // RemainingCapacity
    UPS_REPORT_SIZE(8),
    UPS_REPORT_COUNT(1),
    UPS_LOGICAL_MIN8(0),
    UPS_LOGICAL_MAX8(100),
    UPS_INPUT(UPS_DATA_VAR_ABS_VOL),

    // RunTimeToEmpty - 16位分钟
    UPS_USAGE(0x68),                // RunTimeToEmpty
    UPS_REPORT_SIZE(16),
    UPS_LOGICAL_MIN16(0),
    UPS_LOGICAL_MAX16(32766),
    UPS_INPUT(UPS_DATA_VAR_ABS_VOL),

    // Voltage - 16位 (0.1V单位)
    UPS_USAGE_PAGE(0x84),           // Power Device
    UPS_USAGE(0x84),                // Voltage
    UPS_LOGICAL_MIN16(0),
    UPS_LOGICAL_MAX16(32766),
    UPS_INPUT(UPS_DATA_VAR_ABS_VOL),

    // Current - 16位有符号 (mA单位)
    UPS_USAGE(0x31),                // Current
    UPS_LOGICAL_MIN16(-32768),
    UPS_LOGICAL_MAX16(32767),
    UPS_INPUT(UPS_DATA_VAR_ABS_VOL),

    // PresentStatus - 5位状态 + 3位填充
    UPS_USAGE_PAGE(0x85),           // Battery System
    UPS_USAGE(0xD0),                // ACPresent
    UPS_USAGE(0x44),                // Charging
    UPS_USAGE(0x45),                // Discharging
    UPS_USAGE(0x46),                // FullyCharged
    UPS_USAGE(0x4D),                // BatteryPresent
    UPS_REPORT_SIZE(1),
    UPS_REPORT_COUNT(5),
    UPS_LOGICAL_MIN8(0),
    UPS_LOGICAL_MAX8(1),
    UPS_INPUT(UPS_DATA_VAR_ABS_VOL),
    // 填充到8位
    UPS_REPORT_COUNT(3),
    UPS_INPUT(UPS_CONST_VAR_ABS),

    // === FEATURE 报告 (20字节) ===

    // 字符串索引 (各1字节)
    UPS_USAGE_PAGE(0x84),           // Power Device
    UPS_USAGE(0xFD),                // iManufacturer
    UPS_REPORT_SIZE(8),
    UPS_REPORT_COUNT(1),
    UPS_LOGICAL_MIN8(0),
    UPS_LOGICAL_MAX8(3),
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    UPS_USAGE(0xFE),                // iProduct
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    UPS_USAGE(0xFF),                // iSerialNumber
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    UPS_USAGE(0x01),                // iName
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    // iDeviceChemistry
    UPS_USAGE_PAGE(0x85),           // Battery System
    UPS_USAGE(0x89),                // iDeviceChemistry
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    // CapacityMode - 0=mAh, 1=mWh, 2=%
    UPS_USAGE(0x2C),                // CapacityMode
    UPS_LOGICAL_MIN8(0),
    UPS_LOGICAL_MAX8(2),
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    // Rechargeable
    UPS_USAGE(0x8B),                // Rechargeable
    UPS_LOGICAL_MIN8(0),
    UPS_LOGICAL_MAX8(1),
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    // 容量限制 (各1字节)
    UPS_USAGE(0x8C),                // WarningCapacityLimit
    UPS_LOGICAL_MIN8(0),
    UPS_LOGICAL_MAX8(100),
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    UPS_USAGE(0x29),                // RemainingCapacityLimit
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    // RemainingCapacity (Volatile)
    UPS_USAGE(0x66),                // RemainingCapacity
    UPS_FEATURE(UPS_DATA_VAR_ABS_VOL),

    // FullChargeCapacity, DesignCapacity
    UPS_USAGE(0x67),                // FullChargeCapacity
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    UPS_USAGE(0x83),                // DesignCapacity
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    // RunTimeToEmpty - 16位
    UPS_USAGE(0x68),                // RunTimeToEmpty
    UPS_REPORT_SIZE(16),
    UPS_LOGICAL_MIN16(0),
    UPS_LOGICAL_MAX16(32766),
    UPS_FEATURE(UPS_DATA_VAR_ABS_VOL),

    // RemainingTimeLimit - 16位
    UPS_USAGE(0x2A),                // RemainingTimeLimit
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    // CapacityGranularity1, CapacityGranularity2
    UPS_REPORT_SIZE(8),
    UPS_USAGE(0x8D),                // CapacityGranularity1
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    UPS_USAGE(0x8E),                // CapacityGranularity2
    UPS_FEATURE(UPS_DATA_VAR_ABS),

    // PresentStatus (Feature) - 5位 + 3位填充
    UPS_USAGE(0xD0),                // ACPresent
    UPS_USAGE(0x44),                // Charging
    UPS_USAGE(0x45),                // Discharging
    UPS_USAGE(0x46),                // FullyCharged
    UPS_USAGE(0x4D),                // BatteryPresent
    UPS_REPORT_SIZE(1),
    UPS_REPORT_COUNT(5),
    UPS_LOGICAL_MIN8(0),
    UPS_LOGICAL_MAX8(1),
    UPS_FEATURE(UPS_DATA_VAR_ABS_VOL),
    // 填充到8位
    UPS_REPORT_COUNT(3),
    UPS_FEATURE(UPS_CONST_VAR_ABS),

    UPS_END_COLLECTION,             // End Power Summary
    UPS_END_COLLECTION              // End Application
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
    ITF_NUM_HID = 0,
    ITF_NUM_TOTAL = 1
};

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID 0x81

uint8_t const desc_fs_configuration[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 10)
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return desc_fs_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

enum
{
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
};

#define USB_DESC_STR_MAX_CHARS 64U

static uint16_t _desc_str[USB_DESC_STR_MAX_CHARS + 1];

// 字符串表
static char const *string_desc_arr[] =
{
    [STRID_MANUFACTURER] = "Generic UPS",
    [STRID_PRODUCT]      = "HID Power Device",
    [STRID_SERIAL]       = "UPS123456789"
};

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;

    uint8_t chr_count;

    if (index == 0)
    {
        _desc_str[1] = 0x0409;
        chr_count = 1;
    }
    else
    {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))
        {
            return NULL;
        }

        const char *str = string_desc_arr[index];
        if (str == NULL)
        {
            return NULL;
        }

        chr_count = (uint8_t)strlen(str);
        if (chr_count > USB_DESC_STR_MAX_CHARS)
        {
            chr_count = USB_DESC_STR_MAX_CHARS;
        }

        for (uint8_t i = 0; i < chr_count; i++)
        {
            _desc_str[1 + i] = str[i];
        }
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}
