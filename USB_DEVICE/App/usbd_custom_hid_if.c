#include "usbd_custom_hid_if.h"

/* ============================================================
 * 编译期断言
 * ============================================================ */

#ifndef CONCAT
#define _CONCAT(a, b)                   a ## b
#define CONCAT(a, b)                    _CONCAT(a, b)
#endif

#define STATIC_ASSERT(cond) \
    typedef char CONCAT(static_assert_failed_at_line_, __LINE__)[(cond) ? 1 : -1]

/* ============================================================
 * 常量定义
 * ============================================================ */

#define UPS_REPORT_ID                   (0x01u)

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
 * 报告结构体定义（与 HID Report Descriptor 严格对应）
 * ============================================================ */

STATIC_ASSERT(sizeof(UPS_InputReport_t) == 9);

typedef struct __attribute__((packed))
{
    uint8_t  report_id;                // 0
    uint8_t  iManufacturer;            // 1
    uint8_t  iProduct;                 // 2
    uint8_t  iSerialNumber;            // 3
    uint8_t  iName;                    // 4
    uint8_t  iDeviceChemistry;         // 5
    uint8_t  capacity_mode;            // 6
    uint8_t  rechargeable;             // 7
    uint8_t  warning_capacity_limit;   // 8
    uint8_t  remaining_capacity_limit; // 9
    uint8_t  remaining_capacity;       // 10
    uint8_t  full_charge_capacity;     // 11
    uint8_t  design_capacity;          // 12
    uint16_t runtime_to_empty;         // 13-14
    uint16_t remaining_time_limit;     // 15-16
    uint8_t  capacity_granularity1;    // 17
    uint8_t  capacity_granularity2;    // 18
    uint8_t  present_status;           // 19
} UPS_FeatureReport_t;

STATIC_ASSERT(sizeof(UPS_FeatureReport_t) == 20);

/* ============================================================
 * HID Report Descriptor (存储于 FLASH，const 修饰)
 * ============================================================ */

__ALIGN_BEGIN static const uint8_t CUSTOM_HID_ReportDesc_FS[] __ALIGN_END =
{
    0x05, 0x84,        // USAGE_PAGE (Power Device)
    0x09, 0x04,        // USAGE (UPS)
    0xA1, 0x01,        // COLLECTION (Application)

    0x09, 0x24,        //   USAGE (Power Summary)
    0xA1, 0x02,        //   COLLECTION (Logical)
    0x85, UPS_REPORT_ID, // REPORT_ID (1)

    // === INPUT 报告 ===
    0x05, 0x85,        //     USAGE_PAGE (Battery System)
    0x09, 0x66,        //     USAGE (RemainingCapacity)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x26, 0x64, 0x00,  //     LOGICAL_MAXIMUM (100)
    0x81, 0x82,        //     INPUT (Data,Var,Abs,Volatile)

    0x09, 0x68,        //     USAGE (RunTimeToEmpty)
    0x75, 0x10,        //     REPORT_SIZE (16)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x16, 0x00, 0x00,  //     LOGICAL_MINIMUM (0)
    0x26, 0xFE, 0x7F,  //     LOGICAL_MAXIMUM (32766)
    0x81, 0x82,        //     INPUT (Data,Var,Abs,Volatile)

    0x05, 0x84,        //     USAGE_PAGE (Power Device)
    0x09, 0x84,        //     USAGE (Voltage)
    0x75, 0x10,        //     REPORT_SIZE (16)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x16, 0x00, 0x00,  //     LOGICAL_MINIMUM (0)
    0x26, 0xFE, 0x7F,  //     LOGICAL_MAXIMUM (32766)
    0x81, 0x82,        //     INPUT (Data,Var,Abs,Volatile)

    0x09, 0x31,        //     USAGE (Current)
    0x75, 0x10,        //     REPORT_SIZE (16)
    0x95, 0x01,        //     REPORT_COUNT (1)
    0x16, 0x00, 0x80,  //     LOGICAL_MINIMUM (-32768)
    0x26, 0xFF, 0x7F,  //     LOGICAL_MAXIMUM (32767)
    0x81, 0x82,        //     INPUT (Data,Var,Abs,Volatile)

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
    0x75, 0x01,        //     REPORT_SIZE (1)
    0x95, 0x03,        //     REPORT_COUNT (3)
    0x81, 0x01,        //     INPUT (Constant)

    // === FEATURE 报告 ===
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

    0x05, 0x85,        //     USAGE_PAGE (Battery System)
    0x09, 0x89,        //     USAGE (iDeviceChemistry)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x2C,        //     USAGE (CapacityMode)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x02,        //     LOGICAL_MAXIMUM (2)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x8B,        //     USAGE (Rechargeable)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x25, 0x01,        //     LOGICAL_MAXIMUM (1)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x8C,        //     USAGE (WarningCapacityLimit)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x26, 0x64, 0x00,  //     LOGICAL_MAXIMUM (100)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x29,        //     USAGE (RemainingCapacityLimit)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x66,        //     USAGE (RemainingCapacity)
    0xB1, 0x82,        //     FEATURE (Data,Var,Abs,Volatile)

    0x09, 0x67,        //     USAGE (FullChargeCapacity)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x83,        //     USAGE (DesignCapacity)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x68,        //     USAGE (RunTimeToEmpty)
    0x75, 0x10,        //     REPORT_SIZE (16)
    0x16, 0x00, 0x00,  //     LOGICAL_MINIMUM (0)
    0x26, 0xFE, 0x7F,  //     LOGICAL_MAXIMUM (32766)
    0xB1, 0x82,        //     FEATURE (Data,Var,Abs,Volatile)

    0x75, 0x10,        //     REPORT_SIZE (16)
    0x09, 0x2A,        //     USAGE (RemainingTimeLimit)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x75, 0x08,        //     REPORT_SIZE (8)
    0x09, 0x8D,        //     USAGE (CapacityGranularity1)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

    0x09, 0x8E,        //     USAGE (CapacityGranularity2)
    0xB1, 0x02,        //     FEATURE (Data,Var,Abs)

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
    0x75, 0x01,        //     REPORT_SIZE (1)
    0x95, 0x03,        //     REPORT_COUNT (3)
    0xB1, 0x01,        //     FEATURE (Constant)

    0xC0,              //   END_COLLECTION
    0xC0               // END_COLLECTION
};

/* ============================================================
 * UPS 状态（位域打包，最小 RAM 占用）
 * ============================================================ */

typedef struct
{
    uint8_t remaining_capacity;  // 0-100
    uint8_t ac_present  : 1;
    uint8_t charging    : 1;
    uint8_t discharging : 1;
    uint8_t run_state   : 2;     // UPS_RunState_t
    uint8_t reserved    : 3;
} UPS_State_t;

static UPS_State_t s_ups_state;

/* ============================================================
 * 静态辅助函数（全部 inline，建议编译器展开）
 * ============================================================ */

static inline UPS_RunState_t derive_run_state(uint8_t ac_present, uint8_t capacity)
{
    if (!ac_present)        return UPS_STATE_ON_BATTERY;
    if (capacity >= 100u)   return UPS_STATE_FULLY_CHARGED;
    return UPS_STATE_CHARGING;
}

static inline uint16_t calc_runtime_to_empty(void)
{
    return s_ups_state.ac_present ? 0u
           : (uint16_t)(s_ups_state.remaining_capacity * RUNTIME_MULTIPLIER);
}

static inline int16_t calc_current(void)
{
    switch ((UPS_RunState_t)s_ups_state.run_state)
    {
        case UPS_STATE_ON_BATTERY:   return CURRENT_DISCHARGE_MA;
        case UPS_STATE_CHARGING:     return CURRENT_CHARGE_MA;
        default:                     return 0;
    }
}

static inline uint8_t pack_present_status(void)
{
    uint8_t status = STATUS_BATTERY_PRESENT;

    if (s_ups_state.ac_present)      status |= STATUS_AC_PRESENT;
    if (s_ups_state.charging)        status |= STATUS_CHARGING;
    if (s_ups_state.discharging)     status |= STATUS_DISCHARGING;
    if (s_ups_state.remaining_capacity >= 100u) status |= STATUS_FULLY_CHARGED;

    return status;
}

/* ============================================================
 * USB HID 回调函数
 * ============================================================ */

static int8_t CUSTOM_HID_Init_FS(void)
{
    s_ups_state.remaining_capacity = 100;
    s_ups_state.ac_present         = 1;
    s_ups_state.charging           = 0;
    s_ups_state.discharging        = 1;
    s_ups_state.run_state          = UPS_STATE_ON_BATTERY;
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

void ups_set_status(uint8_t ac_present, uint8_t discharging, uint8_t capacity)
{
    if (capacity > 100u)
    {
        capacity = 100u;
    }

    s_ups_state.ac_present  = ac_present ? 1u : 0u;
    s_ups_state.discharging = discharging ? 1u : 0u;
    s_ups_state.remaining_capacity = capacity;
    s_ups_state.charging    = (ac_present && (capacity < 100u)) ? 1u : 0u;
    s_ups_state.run_state   = (uint8_t)derive_run_state(s_ups_state.ac_present, capacity);
}

UPS_RunState_t ups_get_run_state(void)
{
    return (UPS_RunState_t)s_ups_state.run_state;
}

uint16_t ups_build_input_report(uint8_t *buffer, uint16_t len)
{
    if ((buffer == NULL) || (len < sizeof(UPS_InputReport_t)))
    {
        return 0u;
    }

    UPS_InputReport_t *rpt = (UPS_InputReport_t *)buffer;

    rpt->report_id          = UPS_REPORT_ID;
    rpt->remaining_capacity = s_ups_state.remaining_capacity;
    rpt->runtime_to_empty   = calc_runtime_to_empty();
    rpt->voltage            = VOLTAGE_0_1V;
    rpt->current            = calc_current();
    rpt->present_status     = pack_present_status();

    return sizeof(UPS_InputReport_t);
}

uint16_t ups_build_feature_report(uint8_t *buffer, uint16_t len)
{
    if ((buffer == NULL) || (len < sizeof(UPS_FeatureReport_t)))
    {
        return 0u;
    }

    UPS_FeatureReport_t *rpt = (UPS_FeatureReport_t *)buffer;

    rpt->report_id                = UPS_REPORT_ID;
    rpt->iManufacturer            = IDX_MANUFACTURER;
    rpt->iProduct                 = IDX_PRODUCT;
    rpt->iSerialNumber            = IDX_SERIAL_NUMBER;
    rpt->iName                    = IDX_NAME;
    rpt->iDeviceChemistry         = IDX_DEVICE_CHEMISTRY;
    rpt->capacity_mode            = CAPACITY_MODE_PERCENT;
    rpt->rechargeable             = RECHARGEABLE_YES;
    rpt->warning_capacity_limit   = WARNING_CAPACITY_LIMIT;
    rpt->remaining_capacity_limit = REMAINING_CAPACITY_LIMIT;
    rpt->remaining_capacity       = s_ups_state.remaining_capacity;
    rpt->full_charge_capacity     = FULL_CHARGE_CAPACITY;
    rpt->design_capacity          = DESIGN_CAPACITY;
    rpt->runtime_to_empty         = calc_runtime_to_empty();
    rpt->remaining_time_limit     = REMAINING_TIME_LIMIT_MIN;
    rpt->capacity_granularity1    = CAPACITY_GRANULARITY;
    rpt->capacity_granularity2    = CAPACITY_GRANULARITY;
    rpt->present_status           = pack_present_status();

    return sizeof(UPS_FeatureReport_t);
}

/* ============================================================
 * USB 接口回调结构体
 * ============================================================ */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
    (uint8_t *)CUSTOM_HID_ReportDesc_FS,
    CUSTOM_HID_Init_FS,
    CUSTOM_HID_DeInit_FS,
    CUSTOM_HID_OutEvent_FS
};
