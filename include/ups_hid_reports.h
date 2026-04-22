#ifndef UPS_HID_REPORTS_H_
#define UPS_HID_REPORTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Report IDs (APC 原厂格式)
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

// Builds a HID INPUT report payload (without the leading Report ID byte).
// Returns number of bytes written to buffer.
uint16_t build_hid_input_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen);

// Builds a HID FEATURE report payload (without the leading Report ID byte).
// Returns number of bytes written to buffer.
uint16_t build_hid_feature_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen);

// Packs a date string "MM/DD/YY" into HID Battery ManufacturerDate format.
// Returns 1 on success, 0 on failure.
int pack_hid_date_mmddyy(const char *s, uint16_t *out);

#ifdef __cplusplus
}
#endif

#endif // UPS_HID_REPORTS_H_
