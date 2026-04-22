#ifndef UPS_ADC_H
#define UPS_ADC_H

#include <stdint.h>
#include <stdbool.h>
#include "ups_hid_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ADC 初始化
void ups_adc_init(void);

// ADC 启动采样
void ups_adc_start(void);

// ADC 停止采样
void ups_adc_stop(void);

// ADC 周期任务 (在主循环中调用)
void ups_adc_periodic_task(void);

// 获取电压值 (单位: mV)
uint16_t ups_adc_get_voltage_mv(void);

// 获取电流值 (单位: mA, 有符号)
int16_t ups_adc_get_current_ma(void);

// 获取温度值 (单位: 0.1°C)
uint16_t ups_adc_get_temperature_01c(void);

// 更新 HID 配置中的电气参数 (从ADC读取并更新到HID)
void ups_adc_update_hid_params(void);

// 检查ADC是否就绪
bool ups_adc_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif // UPS_ADC_H
