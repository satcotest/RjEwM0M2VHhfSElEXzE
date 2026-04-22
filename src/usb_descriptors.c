#include "tusb.h"
#include "usb_descriptors.h"

/* A Minimal CDC + HID for MSC mode */

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

    // APC UPS 原厂 VID/PID
    .idVendor = 0x051D,
    .idProduct = 0x0002,
    .bcdDevice = 0x0106,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
// 基于 APC Back-UPS ES 650 (USB ID 051D:0002) 原厂格式
//--------------------------------------------------------------------+

// HID Report Descriptor
// 基于 APC Back-UPS ES 650 原厂格式
uint8_t const desc_hid_report[] =
{
    // === Power Device (UPS) ===
    0x05, 0x84,        // USAGE_PAGE (Power Device)
    0x09, 0x04,        // USAGE (UPS)
    0xA1, 0x01,        // COLLECTION (Application)

    // === Power Summary ===
    0x09, 0x24,        //   USAGE (Power Summary)
    0xA1, 0x02,        //   COLLECTION (Logical)

    // --- Report ID 0x01: iProduct (Feature) ---
    0x85, 0x01,        //     REPORT_ID (0x01)
    0x09, 0xFE,        //     USAGE (iProduct)
    0x79, 0x02,        //     STRING INDEX (2)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x02: iSerialNumber (Feature) ---
    0x85, 0x02,        //     REPORT_ID (0x02)
    0x09, 0xFF,        //     USAGE (iSerialNumber)
    0x79, 0x03,        //     STRING INDEX (3)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x03: iDeviceChemistry (Feature) ---
    0x85, 0x03,        //     REPORT_ID (0x03)
    0x05, 0x85,        //     USAGE_PAGE (Battery System)
    0x09, 0x89,        //     USAGE (iDeviceChemistry)
    0x79, 0x04,        //     STRING INDEX (4)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x04: iOEMInformation (Feature) ---
    0x85, 0x04,        //     REPORT_ID (0x04)
    0x09, 0x8F,        //     USAGE (iOEMInformation)
    0x79, 0x05,        //     STRING INDEX (5)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x05: Rechargeable (Feature) ---
    0x85, 0x05,        //     REPORT_ID (0x05)
    0x09, 0x8B,        //     USAGE (Rechargeable)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x06: CapacityMode (Feature) ---
    0x85, 0x06,        //     REPORT_ID (0x06)
    0x09, 0x2C,        //     USAGE (CapacityMode)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x02,        //     LOGICAL_MAXIMUM (2)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x07: FullChargeCapacity (Feature) ---
    0x85, 0x07,        //     REPORT_ID (0x07)
    0x09, 0x67,        //     USAGE (FullChargeCapacity)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x26, 0x64, 0x00,  //     LOGICAL_MAXIMUM (100)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x08: DesignCapacity (Feature) ---
    0x85, 0x08,        //     REPORT_ID (0x08)
    0x09, 0x83,        //     USAGE (DesignCapacity)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0xC0,              //   END_COLLECTION (PowerSummary)

    // === Report ID 0x0C: RemainingCapacity (Input) ===
    // 完全按照 APC 真机抓包：0C 64 (Report ID + 1字节百分比)
    0x85, 0x0C,        //   REPORT_ID (0x0C)
    0x05, 0x85,        //   USAGE_PAGE (Battery System)
    0x09, 0x66,        //   USAGE (Remaining Capacity)
    0x75, 0x08,        //   REPORT_SIZE (8) - 1 byte 百分比
    0x95, 0x01,        //   REPORT_COUNT (1)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x26, 0x64, 0x00,  //   LOGICAL_MAXIMUM (100)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)

    // === Report ID 0x16: PresentStatus (Input) ===
    // 参考 FreeBSD usbhidctl APC 输出，使用 Battery System Page
    0x85, 0x16,        //   REPORT_ID (0x16)
    0x05, 0x85,        //   USAGE_PAGE (Battery System)
    0x09, 0x6B,        //   USAGE (Present Status)
    0xA1, 0x02,        //   COLLECTION (Logical)
    // 状态位顺序参考 APC 原厂 (FreeBSD usbhidctl)
    0x09, 0x44,        //     USAGE (Charging) - bit 0
    0x09, 0x45,        //     USAGE (Discharging) - bit 1
    0x09, 0xD0,        //     USAGE (AC Present) - bit 2
    0x09, 0x4D,        //     USAGE (Battery Present) - bit 3
    0x09, 0x42,        //     USAGE (Below Remaining Capacity Limit) - bit 4
    0x09, 0x69,        //     USAGE (Shutdown Imminent) - bit 5
    0x09, 0x6C,        //     USAGE (Remaining Time Limit Expired) - bit 6
    0x09, 0x4B,        //     USAGE (Need Replacement) - bit 7
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x01,        //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,        //     REPORT_SIZE (1)
    0x95, 0x08,        //     REPORT_COUNT (8)
    0x81, 0x02,        //     INPUT (Data,Var,Abs)
    0xC0,              //   END_COLLECTION

    0xC0               // END_COLLECTION (Application)
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
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

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return desc_fs_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// String Descriptor Index
enum
{
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
};

// String descriptors
// 使用 USB 字符串描述符格式：长度 + 类型 + UTF-16LE 字符串
#define USB_DESC_STR_MAX_CHARS 64U

static uint16_t _desc_str[USB_DESC_STR_MAX_CHARS + 1]; // +1 for header

// 字符串表
static char const *string_desc_arr[] =
{
    (const char[]){0x09, 0x04}, // 0: 语言 ID (English US)
    "APC",                      // 1: 制造商
    "Back-UPS ES 650 FW:819.v1", // 2: 产品名称 (与 APC 真机一致)
    "4B1719P12345",             // 3: 序列号 (与 APC 真机格式一致)
    "PbAc",                     // 4: 电池化学类型
    "APC"                       // 5: OEM 信息
};

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;

    uint8_t chr_count;

    if (index == STRID_LANGID)
    {
        // 语言 ID 特殊处理
        memcpy(&_desc_str[1], string_desc_arr[STRID_LANGID], 2);
        chr_count = 1;
    }
    else
    {
        // 检查索引是否有效
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))
        {
            return NULL;
        }

        // 转换 ASCII 到 UTF-16LE
        const char *str = string_desc_arr[index];
        chr_count = (uint8_t)strlen(str);

        // 限制最大长度
        if (chr_count > USB_DESC_STR_MAX_CHARS)
        {
            chr_count = USB_DESC_STR_MAX_CHARS;
        }

        // 复制字符
        for (uint8_t i = 0; i < chr_count; i++)
        {
            _desc_str[1 + i] = str[i];
        }
    }

    // 第一个字节是长度，第二个字节是描述符类型
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}
