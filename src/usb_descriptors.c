#include "tusb.h"
#include "usb_descriptors.h"

/*
 * 通用 HID Power Device (UPS) 实现
 * 支持 Windows/Linux/群晖/威联通
 * 参考 STM32_UPS_HID_PowerDevice 和 HIDPowerDevice 项目
 */

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
// 简化版本 - 确保字节对齐
// Report ID 1: Power Summary (Input + Feature)
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] =
{
    // === Power Device (UPS) Application Collection ===
    0x05, 0x84,        // USAGE_PAGE (Power Device)
    0x09, 0x04,        // USAGE (UPS)
    0xA1, 0x01,        // COLLECTION (Application)

    // =====================================================
    // Report ID 1: Power Summary
    // =====================================================
    0x09, 0x24,        //   USAGE (Power Summary)
    0xA1, 0x02,        //   COLLECTION (Logical)
    0x85, 0x01,        //     REPORT_ID (1)

    // === INPUT 报告 (实时数据) ===
    // 所有字段都是 8 位或 16 位，确保字节对齐

    // RemainingCapacity (Input) - 8位百分比
    0x05, 0x85,        //     USAGE_PAGE (Battery System)
    0x09, 0x66,        //     USAGE (RemainingCapacity)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x26, 0x64, 0x00,  //     LOGICAL_MAXIMUM (100)
    0x81, 0x82,        //     INPUT (Data,Var,Abs,Volatile)

    // RunTimeToEmpty (Input) - 16位分钟
    0x09, 0x68,        //     USAGE (RunTimeToEmpty)
    0x75, 0x10,        //     REPORT_SIZE (16)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x16, 0x00, 0x00,  //     LOGICAL_MINIMUM (0)
    0x26, 0xFE, 0x7F,  //     LOGICAL_MAXIMUM (32766)
    0x81, 0x82,        //     INPUT (Data,Var,Abs,Volatile)

    // Voltage (Input) - 16位 (0.1V单位)
    0x05, 0x84,        //     USAGE_PAGE (Power Device)
    0x09, 0x84,        //     USAGE (Voltage)
    0x75, 0x10,        //     REPORT_SIZE (16)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x16, 0x00, 0x00,  //     LOGICAL_MINIMUM (0)
    0x26, 0xFE, 0x7F,  //     LOGICAL_MAXIMUM (32766)
    0x81, 0x82,        //     INPUT (Data,Var,Abs,Volatile)

    // Current (Input) - 16位有符号 (mA单位)
    0x09, 0x31,        //     USAGE (Current)
    0x75, 0x10,        //     REPORT_SIZE (16)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x16, 0x00, 0x80,  //     LOGICAL_MINIMUM (-32768)
    0x26, 0xFF, 0x7F,  //     LOGICAL_MAXIMUM (32767)
    0x81, 0x82,        //     INPUT (Data,Var,Abs,Volatile)

    // PresentStatus (Input) - 简化为 8位标志
    0x05, 0x85,        //     USAGE_PAGE (Battery System)
    0x09, 0xD0,        //     USAGE (ACPresent)
    0x09, 0x44,        //     USAGE (Charging)
    0x09, 0x45,        //     USAGE (Discharging)
    0x09, 0x46,        //     USAGE (FullyCharged)
    0x09, 0x4D,        //     USAGE (BatteryPresent)
    0x75, 0x01,        //     REPORT_SIZE (1)
    0x95, 0x05,        //     REPORT_COUNT (5)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x01,        //     LOGICAL_MAXIMUM (1)
    0x81, 0x82,        //     INPUT (Data,Var,Abs,Volatile)
    // 填充到 8 位
    0x75, 0x01,        //     REPORT_SIZE (1)
    0x95, 0x03,        //     REPORT_COUNT (3)
    0x81, 0x01,        //     INPUT (Constant)

    // === FEATURE 报告 (配置/握手数据) ===
    // 所有字段都是 8 位对齐

    // 字符串索引 (8位)
    0x05, 0x84,        //     USAGE_PAGE (Power Device)
    0x09, 0xFD,        //     USAGE (iManufacturer)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x03,        //     LOGICAL_MAXIMUM (3)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0xFE,        //     USAGE (iProduct)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0xFF,        //     USAGE (iSerialNumber)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x01,        //     USAGE (iName)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // iDeviceChemistry (8位)
    0x05, 0x85,        //     USAGE_PAGE (Battery System)
    0x09, 0x89,        //     USAGE (iDeviceChemistry)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // CapacityMode (8位) - 0=mAh, 1=mWh, 2=%
    0x09, 0x2C,        //     USAGE (CapacityMode)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x02,        //     LOGICAL_MAXIMUM (2)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // Rechargeable (8位)
    0x09, 0x8B,        //     USAGE (Rechargeable)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x01,        //     LOGICAL_MAXIMUM (1)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // 容量限制 (8位)
    0x09, 0x8C,        //     USAGE (WarningCapacityLimit)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x26, 0x64, 0x00,  //     LOGICAL_MAXIMUM (100)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x29,        //     USAGE (RemainingCapacityLimit)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // RemainingCapacity (Feature) - 8位
    0x09, 0x66,        //     USAGE (RemainingCapacity)
    0xB1, 0x82,        //     FEATURE (Data,Var,Abs,Volatile)

    // FullChargeCapacity, DesignCapacity (8位)
    0x09, 0x67,        //     USAGE (FullChargeCapacity)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x83,        //     USAGE (DesignCapacity)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // RunTimeToEmpty (Feature) - 16位
    0x09, 0x68,        //     USAGE (RunTimeToEmpty)
    0x75, 0x10,        //     REPORT_SIZE (16)
    0x16, 0x00, 0x00,  //     LOGICAL_MINIMUM (0)
    0x26, 0xFE, 0x7F,  //     LOGICAL_MAXIMUM (32766)
    0xB1, 0x82,        //     FEATURE (Data,Var,Abs,Volatile)

    // RemainingTimeLimit (Feature) - 16位
    0x75, 0x10,        //     REPORT_SIZE (16)
    0x09, 0x2A,        //     USAGE (RemainingTimeLimit)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // CapacityGranularity1, CapacityGranularity2 (8位)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x09, 0x8D,        //     USAGE (CapacityGranularity1)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x8E,        //     USAGE (CapacityGranularity2)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // PresentStatus (Feature) - 简化为 8位标志
    0x09, 0xD0,        //     USAGE (ACPresent)
    0x09, 0x44,        //     USAGE (Charging)
    0x09, 0x45,        //     USAGE (Discharging)
    0x09, 0x46,        //     USAGE (FullyCharged)
    0x09, 0x4D,        //     USAGE (BatteryPresent)
    0x75, 0x01,        //     REPORT_SIZE (1)
    0x95, 0x05,        //     REPORT_COUNT (5)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x01,        //     LOGICAL_MAXIMUM (1)
    0xB1, 0x82,        //     FEATURE (Data,Var,Abs,Volatile)
    // 填充到 8 位
    0x75, 0x01,        //     REPORT_SIZE (1)
    0x95, 0x03,        //     REPORT_COUNT (3)
    0xB1, 0x01,        //     FEATURE (Constant)

    0xC0,              //   END_COLLECTION (PowerSummary)
    0xC0               // END_COLLECTION (Application)
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
