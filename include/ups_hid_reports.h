#ifndef UPS_HID_REPORTS_H
#define UPS_HID_REPORTS_H

#include <stdint.h>

// 构建 HID Input 报告
uint16_t build_hid_input_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen);

// 构建 HID Feature 报告
uint16_t build_hid_feature_report(uint8_t report_id, uint8_t *buffer, uint16_t reqlen);

#endif // UPS_HID_REPORTS_H
