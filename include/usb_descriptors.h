#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// String descriptor indices
enum {
    USB_STRID_LANGID = 0,
    USB_STRID_MANUFACTURER = 1,
    USB_STRID_PRODUCT = 2,
    USB_STRID_SERIAL = 3,
    USB_STRID_HID_INAME = 4,
    USB_STRID_HID_DEVICE_CHEM = 5,
};

// Buffer for string descriptor conversion (支持最长64字符的USB字符串)
extern uint16_t desc_str[64 + 1];

// Get string count
uint8_t usb_desc_string_count(void);

// Get ASCII string by index
const char *usb_desc_get_string_ascii(uint8_t index);

// Set ASCII string by index
bool usb_desc_set_string_ascii(uint8_t index, const char *str);

#ifdef __cplusplus
 }
#endif

#endif // USB_DESCRIPTORS_H_
