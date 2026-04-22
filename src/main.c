#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "ups_hid_reports.h"
#include "ups_hid_device.h"
#include "ups_hid_config.h"

/* 函数声明 */
void SystemClock_Config(void);
void Error_Handler(void);

/* USB D+ 上拉模拟变量 */
static bool s_usb_dp_held_low = false;

/* UPS 配置 (通用 HID Power Device) */
ups_hid_config_t g_ups_config = UPS_HID_DEFAULT_CONFIG();

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
