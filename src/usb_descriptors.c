/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"
#include "usb_descriptors.h"
#include <stdbool.h>
#include <stdint.h>

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

        // APC 原厂 VID/PID - 匹配 BK650M2-CH
        .idVendor = 0x051D,  // APC
        .idProduct = 0x0002, // Back-UPS
        .bcdDevice = 0x0106,  // Firmware 1.06

        .iManufacturer = 0x01,
        .iProduct = 0x02,
        .iSerialNumber = 0x03,

        .bNumConfigurations = 0x01};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+
//
// 基于 APC Back-UPS CS 500 原厂 Report Descriptor
// 参考: Network UPS Tools (NUT) 调试输出
//
// Windows hidups.sys 初始化流程 (GET_REPORT Feature):
//   ReportID 0x01: iProduct
//   ReportID 0x02: iSerialNumber
//   ReportID 0x03: iDeviceChemistry
//   ReportID 0x04: iOEMInformation
//   ReportID 0x05: Rechargeable
//
// 输入报告 (Input):
//   ReportID 0x0C: RemainingCapacity (电量)
//   ReportID 0x16: PresentStatus (状态)

uint8_t const desc_hid_report[] = {
    // Power Device Page
    0x05, 0x84,  // USAGE_PAGE (Power Device)
    0x09, 0x04,  // USAGE (UPS)
    0xA1, 0x01,  // COLLECTION (Application)

    // === PowerSummary Collection ===
    0x09, 0x24,        //   USAGE (PowerSummary)
    0xA1, 0x00,        //   COLLECTION (Physical)

    // --- Report ID 0x01: iProduct (Feature) ---
    0x85, 0x01,        //     REPORT_ID (0x01)
    0x09, 0xFE,        //     USAGE (iProduct)
    0x79, 0x01,        //     STRING INDEX (1)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,  //     LOGICAL_MAXIMUM (255)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x02: iSerialNumber (Feature) ---
    0x85, 0x02,        //     REPORT_ID (0x02)
    0x09, 0xFF,        //     USAGE (iSerialNumber)
    0x79, 0x02,        //     STRING INDEX (2)
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
    0x79, 0x03,        //     STRING INDEX (3)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x05: Rechargeable (Feature) ---
    0x85, 0x05,        //     REPORT_ID (0x05)
    0x09, 0x8B,        //     USAGE (Rechargeable)
    0x65, 0x00,        //     UNIT (None)
    0x55, 0x00,        //     UNIT EXPONENT (0)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x06: CapacityMode (Feature) ---
    // 关键：设置为 mWh (0x01) 解决 Windows 多电池环境下显示 0% 的问题
    // 参考 HIDPowerDevice Issue #11: https://github.com/abratchik/HIDPowerDevice/issues/11
    0x85, 0x06,        //     REPORT_ID (0x06)
    0x09, 0x2C,        //     USAGE (CapacityMode)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x02,        //     LOGICAL_MAXIMUM (2)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x07: FullChargeCapacity (Feature) ---
    // Windows 需要这个来计算电量百分比 (百分比模式：0-100)
    0x85, 0x07,        //     REPORT_ID (0x07)
    0x09, 0x67,        //     USAGE (FullChargeCapacity)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x26, 0x64, 0x00,  //     LOGICAL_MAXIMUM (100)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    // --- Report ID 0x08: DesignCapacity (Feature) ---
    // Windows 需要这个来计算电量百分比 (百分比模式：0-100)
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
    0x09, 0x66,        //   USAGE (Remaining Capacity)
    0xB1, 0x02,        //   FEATURE (Data,Var,Abs)

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

uint8_t const desc_configuration[] =
    {
        // Config number, interface count, string index, total length, attribute, power in mA
        // set attribute to 0 for bus powered device without remote wakeup
        TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0, 100),

        // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
        TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 200)};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index; // for multiple configurations
    return desc_configuration;
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
    STRID_SERIAL_NUMBER,
    STRID_DEVICE_CHEMISTRY,
    STRID_OEM_INFORMATION
};

// Language ID
#define USB_LANGUAGE_ID 0x0409 // English (US)

// String descriptors data - 匹配真机 BK650M2-CH
static const char *const string_desc_arr[] =
    {
        [STRID_MANUFACTURER] = "American Power Conversion", // 厂商名称
        [STRID_PRODUCT] = "Back-UPS BK650M2-CH FW:294803G-292804G", // 产品名称(匹配真机)
        [STRID_SERIAL_NUMBER] = "9B2216A17431", // 序列号(匹配真机)
        [STRID_DEVICE_CHEMISTRY] = "PbAc", // 电池化学类型 (Lead Acid)
        [STRID_OEM_INFORMATION] = "APC" // OEM 信息
};

// String descriptor buffer
static uint16_t desc_str_buffer[64 + 1];

// Alias for compatibility with header
#define desc_str desc_str_buffer

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;

    uint8_t chr_count;

    if (index == STRID_LANGID)
    {
        desc_str[1] = USB_LANGUAGE_ID;
        chr_count = 1;
    }
    else
    {
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
        {
            return NULL;
        }

        const char *str = string_desc_arr[index];

        // Cap at max chars
        chr_count = (uint8_t)strlen(str);
        if (chr_count > 64)
        {
            chr_count = 64;
        }

        // Convert ASCII string into UTF-16
        for (uint8_t i = 0; i < chr_count; i++)
        {
            desc_str[1 + i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type (0x03)
    desc_str[0] = (uint16_t)((((uint16_t)chr_count) * sizeof(uint16_t)) + sizeof(uint16_t));
    desc_str[0] |= (TUSB_DESC_STRING << 8);  // 设置类型为字符串描述符 (0x03)

    return desc_str;
}
