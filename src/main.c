#include "main.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "usbd_custom_hid_if.h"

/* 函数声明 */
void SystemClock_Config(void);
void Error_Handler(void);

/* 外部USB句柄 */
extern USBD_HandleTypeDef hUsbDeviceFS;

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    /* 软件模拟USB上拉（无电阻可用） */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_Delay(200);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_Delay(300);

    MX_USB_DEVICE_Init();

    uint32_t last_report_time = 0;
    uint8_t capacity = 100;

    while (1)
    {
        uint32_t now = HAL_GetTick();

        // 每1秒发送一次输入报告
        if ((now - last_report_time) >= 1000)
        {
            last_report_time = now;

            // 模拟电量缓慢下降
            if (capacity > 0)
            {
                capacity--;
            }

            // 更新UPS状态
            ups_set_status(1, 0, capacity);

            // 构建INPUT报告数据并通过中断端点发送
            uint8_t report[16];
            uint16_t len = ups_build_input_report(report, sizeof(report));
            if (len > 0)
            {
                USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, len);
            }
        }

        HAL_Delay(10);
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
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
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
