#include "usbd_custom_hid_if.h"

// APC UPS 原厂格式 Report Descriptor
// Report ID 0x0C: 电量报告 (2字节: 0C 64)
// Report ID 0x16: 状态报告 (3字节: 16 0C 00)
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[] __ALIGN_END =
{
    // Power Device Page
    0x05, 0x84,  // USAGE_PAGE (Power Device)
    0x09, 0x04,  // USAGE (UPS)
    0xA1, 0x01,  // COLLECTION (Application)

    // Report ID 0x0C - 电量报告
    0x85, 0x0C,  //   REPORT_ID (0x0C)
    0x09, 0x66,  //   USAGE (Remaining Capacity)
    0x15, 0x00,  //   LOGICAL_MINIMUM (0)
    0x26, 0x64, 0x00,  //   LOGICAL_MAXIMUM (100)
    0x75, 0x08,  //   REPORT_SIZE (8)
    0x95, 0x01,  //   REPORT_COUNT (1)
    0x81, 0x02,  //   INPUT (Data,Var,Abs)

    // Report ID 0x16 - 状态报告
    0x85, 0x16,  //   REPORT_ID (0x16)
    0x09, 0x6B,  //   USAGE (Present Status)
    0xA1, 0x02,  //   COLLECTION (Logical)
    0x09, 0xD0,  //     USAGE (AC Present)
    0x09, 0x44,  //     USAGE (Charging)
    0x09, 0x45,  //     USAGE (Discharging)
    0x09, 0x46,  //     USAGE (Fully Charged)
    0x09, 0x4B,  //     USAGE (Need Replacement)
    0x09, 0x42,  //     USAGE (Below Remaining Capacity Limit)
    0x09, 0x4D,  //     USAGE (Battery Present)
    0x09, 0x73,  //     USAGE (Overload)
    0x15, 0x00,  //     LOGICAL_MINIMUM (0)
    0x25, 0x01,  //     LOGICAL_MAXIMUM (1)
    0x75, 0x01,  //     REPORT_SIZE (1)
    0x95, 0x08,  //     REPORT_COUNT (8)
    0x81, 0x02,  //     INPUT (Data,Var,Abs)
    0xC0,        //   END_COLLECTION
    0x81, 0x01,  //   INPUT (Const) - 填充到16位对齐

    0xC0         // END_COLLECTION
};

// 空实现（和CubeMX默认一致），无任何多余变量
static int8_t CUSTOM_HID_Init_FS(void)
{
  return (USBD_OK);
}

static int8_t CUSTOM_HID_DeInit_FS(void)
{
  return (USBD_OK);
}

// 核心修复：删掉所有cmd相关代码，还原为空实现，彻底消除未初始化警告
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)
{
  UNUSED(event_idx);
  UNUSED(state);
  /* 如果你暂时不需要处理主机下发的指令，这里留空即可 */
  return (USBD_OK);
}

// HID接口函数集（CubeMX默认结构）
USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS
};
