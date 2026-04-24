#include "usbd_custom_hid_if.h"

// APC BK650M2-CH 原厂标准HID报告描述符 (52字节)
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[] __ALIGN_END =
{
    0x05, 0x84, 0x09, 0x04, 0xA1, 0x01, 0x85, 0x01,
    0x09, 0x1D, 0xA1, 0x02, 0x09, 0xD1, 0xA1, 0x02,
    0x09, 0xD0, 0x09, 0xDD, 0x09, 0x42, 0x15, 0x00,
    0x25, 0x01, 0x75, 0x01, 0x95, 0x03, 0x81, 0x02,
    0x95, 0x05, 0x81, 0x03, 0xC0, 0x09, 0x66, 0x15,
    0x00, 0x26, 0x64, 0x00, 0x75, 0x08, 0x95, 0x01,
    0x81, 0x02, 0xC0, 0xC0
};

static int8_t CUSTOM_HID_Init_FS(void)
{
  return (USBD_OK);
}

static int8_t CUSTOM_HID_DeInit_FS(void)
{
  return (USBD_OK);
}

static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)
{
  UNUSED(event_idx);
  UNUSED(state);
  return (USBD_OK);
}

// UPS状态变量
static uint8_t s_remaining_capacity = 100;
static uint8_t s_ac_present = 1;
static uint8_t s_charging = 0;
static uint8_t s_discharging = 1;
static uint8_t s_battery_present = 1;

// 设置UPS状态
void ups_set_status(uint8_t ac_present, uint8_t discharging, uint8_t capacity)
{
    s_ac_present = ac_present;
    s_discharging = discharging;
    s_remaining_capacity = capacity;
    s_charging = ac_present && (capacity < 100);
}

// 构建输入报告 - 8字节格式
// [RemainingCapacity(1)] [RunTimeToEmpty(2)] [Voltage(2)] [Current(2)] [PresentStatus(1)]
uint16_t ups_build_input_report(uint8_t *buffer, uint16_t len)
{
    if (len < 8 || buffer == NULL)
    {
        return 0;
    }

    uint8_t idx = 0;

    // RemainingCapacity (1字节)
    buffer[idx++] = s_remaining_capacity;

    // RunTimeToEmpty (2字节) - 估算值: 电量% * 10分钟
    uint16_t runtime = s_remaining_capacity * 10;
    buffer[idx++] = runtime & 0xFF;
    buffer[idx++] = (runtime >> 8) & 0xFF;

    // Voltage (2字节) - 12.0V = 120 (0.1V单位)
    uint16_t voltage = 120;
    buffer[idx++] = voltage & 0xFF;
    buffer[idx++] = (voltage >> 8) & 0xFF;

    // Current (2字节有符号) - 充电为正，放电为负
    int16_t current = s_charging ? 1000 : (s_discharging ? -500 : 0);
    buffer[idx++] = current & 0xFF;
    buffer[idx++] = ((uint16_t)current >> 8) & 0xFF;

    // PresentStatus (1字节)
    // bit 0: AC Present, bit 1: Charging, bit 2: Discharging, bit 3: FullyCharged, bit 4: BatteryPresent
    uint8_t status = 0;
    if (s_ac_present) status |= 0x01;
    if (s_charging) status |= 0x02;
    if (s_discharging) status |= 0x04;
    if (s_remaining_capacity >= 100) status |= 0x08;
    if (s_battery_present) status |= 0x10;
    buffer[idx++] = status;

    return idx;
}

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS
};
