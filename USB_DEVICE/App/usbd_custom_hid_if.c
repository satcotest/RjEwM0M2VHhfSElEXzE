#include "usbd_custom_hid_if.h"

// HID Power Device UPS 报告描述符
// 参考 TinyUSB try-tinyusb 分支的 descriptor 结构
// 单一 Report ID (1)，同时包含 INPUT 和 FEATURE 报告
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[] __ALIGN_END =
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

// 设置UPS状态
void ups_set_status(uint8_t ac_present, uint8_t discharging, uint8_t capacity)
{
    s_ac_present = ac_present;
    s_discharging = discharging;
    s_remaining_capacity = capacity;
    s_charging = ac_present && (capacity < 100);
}

// 打包 PresentStatus 位字段
static uint8_t pack_present_status(void)
{
    uint8_t status = 0;
    if (s_ac_present)       status |= 0x01;  // bit 0: ACPresent
    if (s_charging)         status |= 0x02;  // bit 1: Charging
    if (s_discharging)      status |= 0x04;  // bit 2: Discharging
    if (s_remaining_capacity >= 100) status |= 0x08;  // bit 3: FullyCharged
    status |= 0x10;                          // bit 4: BatteryPresent
    return status;
}

// 构建INPUT报告 - Report ID 1
// 格式: [ReportID(1)] [RemainingCapacity(1)] [RunTimeToEmpty(2)] [Voltage(2)] [Current(2)] [PresentStatus(1)]
// 总计: 9字节
uint16_t ups_build_input_report(uint8_t *buffer, uint16_t len)
{
    if (len < 9 || buffer == NULL)
    {
        return 0;
    }

    uint8_t idx = 0;

    // Report ID
    buffer[idx++] = 0x01;

    // RemainingCapacity (1字节) - 百分比
    buffer[idx++] = s_remaining_capacity;

    // RunTimeToEmpty (2字节) - 分钟
    uint16_t runtime = 0;
    if (!s_ac_present)
    {
        runtime = (uint16_t)(s_remaining_capacity * 6); // 模拟: 100% = 600分钟
    }
    buffer[idx++] = LOBYTE(runtime);
    buffer[idx++] = HIBYTE(runtime);

    // Voltage (2字节) - 0.1V单位
    uint16_t voltage = 1200; // 120.0V
    buffer[idx++] = LOBYTE(voltage);
    buffer[idx++] = HIBYTE(voltage);

    // Current (2字节) - mA单位
    int16_t current = 0;
    if (s_discharging)
    {
        current = -500; // 放电500mA
    }
    else if (s_charging)
    {
        current = 1000; // 充电1000mA
    }
    buffer[idx++] = LOBYTE((uint16_t)current);
    buffer[idx++] = HIBYTE((uint16_t)current);

    // PresentStatus (1字节)
    buffer[idx++] = pack_present_status();

    return idx;
}

// 构建FEATURE报告 - Report ID 1
// 格式:
//   [ReportID(1)] [iManufacturer(1)] [iProduct(1)] [iSerialNumber(1)] [iName(1)] [iDeviceChemistry(1)]
//   [CapacityMode(1)] [Rechargeable(1)]
//   [WarningCapacityLimit(1)] [RemainingCapacityLimit(1)]
//   [RemainingCapacity(1)] [FullChargeCapacity(1)] [DesignCapacity(1)]
//   [RunTimeToEmpty(2)] [RemainingTimeLimit(2)]
//   [CapacityGranularity1(1)] [CapacityGranularity2(1)]
//   [PresentStatus(1)]
// 总计: 21字节
uint16_t ups_build_feature_report(uint8_t *buffer, uint16_t len)
{
    if (len < 21 || buffer == NULL)
    {
        return 0;
    }

    uint8_t idx = 0;

    // Report ID
    buffer[idx++] = 0x01;

    // 字符串索引 (各1字节)
    buffer[idx++] = 0x01;  // iManufacturer
    buffer[idx++] = 0x02;  // iProduct
    buffer[idx++] = 0x03;  // iSerialNumber
    buffer[idx++] = 0x02;  // iName
    buffer[idx++] = 0x01;  // iDeviceChemistry

    // CapacityMode (1字节) - 2=百分比
    buffer[idx++] = 0x02;

    // Rechargeable (1字节)
    buffer[idx++] = 0x01;

    // 容量限制 (各1字节)
    buffer[idx++] = 20;   // WarningCapacityLimit
    buffer[idx++] = 10;   // RemainingCapacityLimit

    // 容量值 (各1字节)
    buffer[idx++] = s_remaining_capacity;  // RemainingCapacity
    buffer[idx++] = 100;  // FullChargeCapacity
    buffer[idx++] = 100;  // DesignCapacity

    // RunTimeToEmpty (2字节)
    uint16_t runtime = 0;
    if (!s_ac_present)
    {
        runtime = (uint16_t)(s_remaining_capacity * 6);
    }
    buffer[idx++] = LOBYTE(runtime);
    buffer[idx++] = HIBYTE(runtime);

    // RemainingTimeLimit (2字节)
    buffer[idx++] = 0x78;  // 120 minutes LSB
    buffer[idx++] = 0x00;  // 120 minutes MSB

    // 容量粒度 (各1字节)
    buffer[idx++] = 1;    // CapacityGranularity1
    buffer[idx++] = 1;    // CapacityGranularity2

    // PresentStatus (1字节)
    buffer[idx++] = pack_present_status();

    return idx;
}

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS
};
