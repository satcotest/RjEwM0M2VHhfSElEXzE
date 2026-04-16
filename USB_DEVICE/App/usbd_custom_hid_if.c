/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_custom_hid_if.c
  * @version        : v2.0_Cube
  * @brief          : USB Device Custom HID interface file.
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_custom_hid_if.h"
#include "stm32f1xx_hal.h"
#include <string.h>

/* 外部声明USB设备句柄（必须和usb_device.c中的一致） */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* APC BK650M2-CH 原厂标准HID报告描述符 (39字节) */
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

/* ========== 核心修复1：不再重复定义结构体（直接用.h中声明的） ========== */
/* 全局UPS状态变量（仅在这里定义，.h中用extern声明） */
APC_UPS_InputReport_t apc_ups_status = {
    .status_bits.mains_on = 1,    // 默认市电正常
    .status_bits.fault = 0,       // 无故障
    .status_bits.battery_low = 0, // 电池正常
    .status_bits.reserved = 0,    // 保留位清零
    .percentage = 95              // 默认电量95%
};

/* ========== CUSTOM HID核心回调 ========== */
static int8_t CUSTOM_HID_Init_FS(void)
{
  /* 初始化时清空状态（可选） */
  memset(&apc_ups_status, 0, sizeof(APC_UPS_InputReport_t));
  apc_ups_status.status_bits.mains_on = 1;
  apc_ups_status.percentage = 95;
  return (USBD_OK);
}

static int8_t CUSTOM_HID_DeInit_FS(void)
{
  return (USBD_OK);
}

/* 核心修复2：正确读取主机下发的输出报告（替换不存在的USBD_CUSTOM_HID_GetReport） */
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)
{
  UNUSED(event_idx);
  UNUSED(state);
  
  /* 读取主机下发的指令（Cube官方写法：从USB接收缓冲区读数据） */
  APC_UPS_OutputReport_t cmd;
  /* 核心：hUsbDeviceFS.pClassData 是CUSTOM HID的私有数据缓冲区 */
  USBD_CUSTOM_HID_HandleTypeDef *hhid = (USBD_CUSTOM_HID_HandleTypeDef *)hUsbDeviceFS.pClassData;
  if (hhid != NULL) {
      /* 复制主机下发的1字节指令到cmd结构体 */
      memcpy(&cmd.cmd, hhid->Report_buf, sizeof(APC_UPS_OutputReport_t));
  }
  
  /* 解析APC原厂指令 */
  switch(cmd.cmd) {
    case 0x01: // 自检指令
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // 示例：LED闪烁表示自检
        break;
    case 0x02: // 关机指令
        // 实现UPS关机逻辑（如延时切断外设电源）
        break;
    case 0x03: // 查询状态指令 → 立即上报当前状态
        APC_UPS_SendStatus();
        break;
    default:
        break;
  }
  
  return (USBD_OK);
}

/* ========== 自定义函数：上报UPS状态到主机 ========== */
uint8_t APC_UPS_SendStatus(void)
{
  /* 发送输入报告（Report ID=0x01，数据=2字节状态） */
  return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, 
                                    (uint8_t*)&apc_ups_status, 
                                    sizeof(APC_UPS_InputReport_t));
}

/* ========== CUSTOM HID接口函数集（必须保留）========== */
USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS =
{
  CUSTOM_HID_ReportDesc_FS,
  CUSTOM_HID_Init_FS,
  CUSTOM_HID_DeInit_FS,
  CUSTOM_HID_OutEvent_FS
};
