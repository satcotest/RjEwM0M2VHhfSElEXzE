#include "usbd_custom_hid_if.h"

/* ============================================================
 * 常量定义
 * ============================================================ */

#define UPS_REPORT_ID                   (0x01u)

// INPUT 报告大小 (字节)
#define UPS_INPUT_REPORT_SIZE           (9u)

// FEATURE 报告大小 (字节)
#define UPS_FEATURE_REPORT_SIZE         (21u)

// 状态位掩码 (PresentStatus)
#define STATUS_AC_PRESENT               (0x01u)  // bit 0
#define STATUS_CHARGING                 (0x02u)  // bit 1
#define STATUS_DISCHARGING              (0x04u)  // bit 2
#define STATUS_FULLY_CHARGED            (0x08u)  // bit 3
#define STATUS_BATTERY_PRESENT          (0x10u)  // bit 4

// 固定参数值
#define CAPACITY_MODE_PERCENT           (0x02u)
#define RECHARGEABLE_YES                (0x01u)
#define WARNING_CAPACITY_LIMIT          (20u)
#define REMAINING_CAPACITY_LIMIT        (10u)
#define FULL_CHARGE_CAPACITY            (100u)
#define DESIGN_CAPACITY                 (100u)
#define REMAINING_TIME_LIMIT_MIN        (120u)
#define CAPACITY_GRANULARITY            (1u)

// 电气参数
#define VOLTAGE_0_1V                    (1200u)     // 120.0V
#define CURRENT_DISCHARGE_MA            (-500)      // 放电 500mA
#define CURRENT_CHARGE_MA               (1000)      // 充电 1000mA
#define RUNTIME_MULTIPLIER              (6u)        // 100% = 600分钟

// 字符串索引
#define IDX_MANUFACTURER                (0x01u)
#define IDX_PRODUCT                     (0x02u)
#define IDX_SERIAL_NUMBER               (0x03u)
#define IDX_NAME                        (0x02u)
#define IDX_DEVICE_CHEMISTRY            (0x01u)

/* ============================================================
 * HID Report Descriptor
 * ============================================================ */

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
    0x85, UPS_REPORT_ID, // REPORT_ID (1)

    // === INPUT 报告 (实时数据) ===

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

/* ============================================================
 * UPS 状态结构体
 * ============================================================ */

typedef struct
{
    uint8_t remaining_capacity;  // 0-100
    uint8_t ac_present;          // 0 或 1
    uint8_t charging;            // 0 或 1
    uint8_t discharging;         // 0 或 1
} UPS_State_t;

static UPS_State_t s_ups_state =
{
    .remaining_capacity = 100,
    .ac_present         = 1,
    .charging           = 0,
    .discharging        = 1
};

/* ============================================================
 * 静态辅助函数
 * ============================================================ */

/**
 * @brief 计算运行时间 (分钟)
 * @return 如果AC连接返回0，否则根据剩余容量计算
 */
static inline uint16_t calc_runtime_to_empty(void)
{
    if (s_ups_state.ac_present)
    {
        return 0u;
    }
    return (uint16_t)(s_ups_state.remaining_capacity * RUNTIME_MULTIPLIER);
}

/**
 * @brief 计算当前电流 (mA)
 * @return 有符号电流值
 */
static inline int16_t calc_current(void)
{
    if (s_ups_state.discharging)
    {
        return CURRENT_DISCHARGE_MA;
    }
    if (s_ups_state.charging)
    {
        return CURRENT_CHARGE_MA;
    }
    return 0;
}

/**
 * @brief 打包 PresentStatus 位字段
 * @return 8位状态标志
 */
static uint8_t pack_present_status(void)
{
    uint8_t status = STATUS_BATTERY_PRESENT;

    if (s_ups_state.ac_present)
    {
        status |= STATUS_AC_PRESENT;
    }
    if (s_ups_state.charging)
    {
        status |= STATUS_CHARGING;
    }
    if (s_ups_state.discharging)
    {
        status |= STATUS_DISCHARGING;
    }
    if (s_ups_state.remaining_capacity >= 100)
    {
        status |= STATUS_FULLY_CHARGED;
    }
    return status;
}

/* ============================================================
 * USB HID 回调函数
 * ============================================================ */

static int8_t CUSTOM_HID_Init_FS(void)
{
    return USBD_OK;
}

static int8_t CUSTOM_HID_DeInit_FS(void)
{
    return USBD_OK;
}

static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)
{
    UNUSED(event_idx);
    UNUSED(state);
    return USBD_OK;
}

/* ============================================================
 * 公共接口
 * ============================================================ */

/**
 * @brief 设置UPS状态
 * @param ac_present   1=AC连接, 0=电池供电
 * @param discharging  1=放电中, 0=未放电
 * @param capacity     剩余容量百分比 (0-100)
 */
void ups_set_status(uint8_t ac_present, uint8_t discharging, uint8_t capacity)
{
    // 限制容量范围
    if (capacity > 100u)
    {
        capacity = 100u;
    }

    s_ups_state.ac_present      = ac_present ? 1u : 0u;
    s_ups_state.discharging     = discharging ? 1u : 0u;
    s_ups_state.remaining_capacity = capacity;
    s_ups_state.charging        = (ac_present && (capacity < 100u)) ? 1u : 0u;
}

/**
 * @brief 构建 INPUT 报告
 *
 * 格式:
 *   [ReportID(1)] [RemainingCapacity(1)] [RunTimeToEmpty(2)] [Voltage(2)] [Current(2)] [PresentStatus(1)]
 * 总计: 9字节
 *
 * @param buffer 输出缓冲区
 * @param len    缓冲区大小
 * @return 实际写入的字节数，失败返回0
 */
uint16_t ups_build_input_report(uint8_t *buffer, uint16_t len)
{
    if ((buffer == NULL) || (len < UPS_INPUT_REPORT_SIZE))
    {
        return 0u;
    }

    const uint16_t runtime  = calc_runtime_to_empty();
    const int16_t  current  = calc_current();
    uint8_t idx = 0;

    buffer[idx++] = UPS_REPORT_ID;
    buffer[idx++] = s_ups_state.remaining_capacity;
    buffer[idx++] = LOBYTE(runtime);
    buffer[idx++] = HIBYTE(runtime);
    buffer[idx++] = LOBYTE(VOLTAGE_0_1V);
    buffer[idx++] = HIBYTE(VOLTAGE_0_1V);
    buffer[idx++] = LOBYTE((uint16_t)current);
    buffer[idx++] = HIBYTE((uint16_t)current);
    buffer[idx++] = pack_present_status();

    return idx;
}

/**
 * @brief 构建 FEATURE 报告
 *
 * 格式:
 *   [ReportID(1)] [iManufacturer(1)] [iProduct(1)] [iSerialNumber(1)] [iName(1)] [iDeviceChemistry(1)]
 *   [CapacityMode(1)] [Rechargeable(1)]
 *   [WarningCapacityLimit(1)] [RemainingCapacityLimit(1)]
 *   [RemainingCapacity(1)] [FullChargeCapacity(1)] [DesignCapacity(1)]
 *   [RunTimeToEmpty(2)] [RemainingTimeLimit(2)]
 *   [CapacityGranularity1(1)] [CapacityGranularity2(1)]
 *   [PresentStatus(1)]
 * 总计: 21字节
 *
 * @param buffer 输出缓冲区
 * @param len    缓冲区大小
 * @return 实际写入的字节数，失败返回0
 */
uint16_t ups_build_feature_report(uint8_t *buffer, uint16_t len)
{
    if ((buffer == NULL) || (len < UPS_FEATURE_REPORT_SIZE))
    {
        return 0u;
    }

    const uint16_t runtime = calc_runtime_to_empty();
    uint8_t idx = 0;

    buffer[idx++] = UPS_REPORT_ID;
    buffer[idx++] = IDX_MANUFACTURER;
    buffer[idx++] = IDX_PRODUCT;
    buffer[idx++] = IDX_SERIAL_NUMBER;
    buffer[idx++] = IDX_NAME;
    buffer[idx++] = IDX_DEVICE_CHEMISTRY;
    buffer[idx++] = CAPACITY_MODE_PERCENT;
    buffer[idx++] = RECHARGEABLE_YES;
    buffer[idx++] = WARNING_CAPACITY_LIMIT;
    buffer[idx++] = REMAINING_CAPACITY_LIMIT;
    buffer[idx++] = s_ups_state.remaining_capacity;
    buffer[idx++] = FULL_CHARGE_CAPACITY;
    buffer[idx++] = DESIGN_CAPACITY;
    buffer[idx++] = LOBYTE(runtime);
    buffer[idx++] = HIBYTE(runtime);
    buffer[idx++] = LOBYTE(REMAINING_TIME_LIMIT_MIN);
    buffer[idx++] = HIBYTE(REMAINING_TIME_LIMIT_MIN);
    buffer[idx++] = CAPACITY_GRANULARITY;
    buffer[idx++] = CAPACITY_GRANULARITY;
    buffer[idx++] = pack_present_status();

    return idx;
}

/* ============================================================
 * USB 接口回调结构体
 * ============================================================ */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
    CUSTOM_HID_ReportDesc_FS,
    CUSTOM_HID_Init_FS,
    CUSTOM_HID_DeInit_FS,
    CUSTOM_HID_OutEvent_FS
};
