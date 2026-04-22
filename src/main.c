#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "ups_hid_reports.h"
#include "ups_hid_device.h"
#include "ups_data.h"

/* 函数声明 */
void SystemClock_Config(void);
void Error_Handler(void);

/* USB D+ 上拉模拟变量 */
static bool s_usb_dp_held_low = false;

/* UPS 状态变量 - APC 原厂格式
 * 状态字节 0x0C = 0000 1100:
 *   bit2: 1 = 市电正常/充电中
 *   bit3: 1 = 无故障/正常状态
 */
ups_present_status_t g_power_summary_present_status = {
    .ac_present = true,
    .charging = true,
    .discharging = false,
    .fully_charged = true,
    .need_replacement = false,
    .below_remaining_capacity_limit = false,
    .battery_present = true,
    .overload = false,
    .shutdown_imminent = false,
};

ups_summary_t g_power_summary = {
    .rechargeable = true,
    .capacity_mode = 2U,
    .design_capacity = 100U,
    .full_charge_capacity = 100U,
    .warning_capacity_limit = 20U,
    .remaining_capacity_limit = 10U,
    .i_device_chemistry = 0x05U,
    .capacity_granularity_1 = 1U,
    .capacity_granularity_2 = 1U,
    .i_manufacturer_2bit = 1U,
    .i_product_2bit = 2U,
    .i_serial_number_2bit = 3U,
    .i_name_2bit = 2U,
};

ups_battery_t g_battery = {
    .battery_voltage = 0,
    .battery_current = 0,
    .config_voltage = 0,
    .run_time_to_empty_s = 3600,
    .remaining_time_limit_s = 120,
    .temperature = 0,
    .manufacturer_date = 0,
    .remaining_capacity = 100,  // APC 原厂格式: 100% = 0x64
};

ups_input_t g_input = {
    .voltage = 0,
    .frequency = 0,
    .config_voltage = 0,
    .low_voltage_transfer = 0,
    .high_voltage_transfer = 0,
};

ups_output_t g_output = {
    .percent_load = 0,
    .config_active_power = 0,
    .config_voltage = 0,
    .voltage = 0,
    .current = 0,
    .frequency = 0,
};

static void usb_dp_hold_low(bool hold)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_12;

    if (hold)
    {
        gpio.Mode = GPIO_MODE_OUTPUT_OD;
        gpio.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOA, &gpio);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
        s_usb_dp_held_low = true;
    }
    else
    {
        gpio.Mode = GPIO_MODE_INPUT;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &gpio);
        s_usb_dp_held_low = false;
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    /* 启用 USB 时钟 */
    __HAL_RCC_USB_CLK_ENABLE();

    /* 启用 USB 中断 */
    HAL_NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);

    /* 软件模拟USB上拉：先把D+拉低 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    usb_dp_hold_low(true);
    HAL_Delay(200);

    /* 初始化 TinyUSB */
    tusb_init();
    usb_dp_hold_low(false);
    HAL_Delay(5);
    (void)tud_connect();

    while (1)
    {
        tud_task();
        ups_hid_periodic_task();
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_RCC_EnableCSS();
}

void Error_Handler(void)
{
    __disable_irq();
    while(1){}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line){}
#endif
