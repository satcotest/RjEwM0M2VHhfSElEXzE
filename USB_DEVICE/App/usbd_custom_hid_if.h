/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.h
  * @version        : v2.0_Cube
  * @brief          : Header for usbd_custom_hid_if.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CUSTOM_HID_IF_H__
#define __USBD_CUSTOM_HID_IF_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_customhid.h"

/* USER CODE BEGIN INCLUDE */
#include <stdint.h>  // 补充uint8_t等类型定义
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief For Usb device.
  * @{
  */

/** @defgroup USBD_CUSTOM_HID USBD_CUSTOM_HID
  * @brief Usb custom human interface device module.
  * @{
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Defines USBD_CUSTOM_HID_Exported_Defines
  * @brief Defines.
  * @{
  */

/* USER CODE BEGIN EXPORTED_DEFINES */
/* USER CODE END EXPORTED_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Types USBD_CUSTOM_HID_Exported_Types
  * @brief Types.
  * @{
  */

/* USER CODE BEGIN EXPORTED_TYPES */
// APC BK650M2-CH UPS HID输入报告结构体（STM32→主机，2字节）
typedef struct {
    union {
        uint8_t status_byte;  // 低3位=状态位，高5位=保留（固定0）
        struct {
            uint8_t mains_on    : 1; // Bit0: 1=市电正常, 0=市电断开
            uint8_t fault       : 1; // Bit1: 1=故障, 0=无故障
            uint8_t battery_low : 1; // Bit2: 1=电池低, 0=电池正常
            uint8_t reserved    : 5; // Bit3~7: 保留位，固定为0
        } status_bits;
    };
    uint8_t percentage;        // Byte1: 电池电量/负载百分比（0~100）
} APC_UPS_InputReport_t;

// APC BK650M2-CH UPS HID输出报告结构体（主机→STM32，1字节）
typedef struct {
    uint8_t cmd;               // 指令码：0=无指令, 1=自检, 2=关机, 3=查询状态
} APC_UPS_OutputReport_t;
/* USER CODE END EXPORTED_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Macros USBD_CUSTOM_HID_Exported_Macros
  * @brief Aliases.
  * @{
  */

/* USER CODE BEGIN EXPORTED_MACRO */
/* USER CODE END EXPORTED_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
  * @brief Public variables.
  * @{
  */

/** CUSTOMHID Interface callback. */
extern USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS;

/* USER CODE BEGIN EXPORTED_VARIABLES */
// 全局UPS状态变量（仅声明，定义在.c文件中）
extern APC_UPS_InputReport_t apc_ups_status;
/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CUSTOM_HID_Exported_FunctionsPrototype USBD_CUSTOM_HID_Exported_FunctionsPrototype
  * @brief Public functions declaration.
  * @{
  */

/* USER CODE BEGIN EXPORTED_FUNCTIONS */
// 自定义函数：向主机上报UPS状态（返回值：USBD_OK/USBD_FAIL）
uint8_t APC_UPS_SendStatus(void);
/* USER CODE END EXPORTED_FUNCTIONS */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CUSTOM_HID_IF_H__ */
