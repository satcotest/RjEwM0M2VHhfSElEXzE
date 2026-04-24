#include "main.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "usbd_custom_hid_if.h"

/* ============================================================
 * 常量定义
 * ============================================================ */

#define REPORT_INTERVAL_MS      (1000u)     // 报告发送间隔
#define TICKS_PER_PERCENT       (10u)       // 电量每变化1%需要的周期数
#define USB_DP_PULLUP_DELAY_MS  (200u)      // USB D+ 上拉延迟
#define USB_ENUM_DELAY_MS       (300u)      // USB 枚举等待时间

/* ============================================================
 * 函数声明
 * ============================================================ */

void SystemClock_Config(void);
void Error_Handler(void);

static void usb_dp_pullup_init(void);
static void ups_simulate_step(uint8_t *capacity, uint8_t ac_present);

/* ============================================================
 * 外部USB句柄
 * ============================================================ */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* ============================================================
 * 主函数
 * ============================================================ */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    usb_dp_pullup_init();
    MX_USB_DEVICE_Init();

    uint32_t last_report_time = HAL_GetTick();
    uint8_t  capacity         = 100;
    uint8_t  ac_present       = 1;
    uint8_t  tick_count       = 0;

    while (1)
    {
        uint32_t now = HAL_GetTick();

        if ((now - last_report_time) >= REPORT_INTERVAL_MS)
        {
            last_report_time = now;
            tick_count++;

            if (tick_count >= TICKS_PER_PERCENT)
            {
                tick_count = 0;
                ups_simulate_step(&capacity, ac_present);
            }

            uint8_t report[sizeof(UPS_InputReport_t)];
            uint16_t len = ups_build_input_report(report, sizeof(report));
            if (len > 0)
            {
                USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, len);
            }
        }

        HAL_Delay(10);
    }
}

/* ============================================================
 * USB D+ 软件上拉初始化
 * ============================================================ */

static void usb_dp_pullup_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct =
    {
        .Pin   = GPIO_PIN_12,
        .Mode  = GPIO_MODE_OUTPUT_PP,
        .Pull  = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_Delay(USB_DP_PULLUP_DELAY_MS);

    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_Delay(USB_ENUM_DELAY_MS);
}

/* ============================================================
 * UPS 电量模拟步进
 * ============================================================ */

static void ups_simulate_step(uint8_t *capacity, uint8_t ac_present)
{
    if (!ac_present && *capacity > 0)
    {
        (*capacity)--;
        ups_set_status(0, 1, *capacity);
    }
    else if (ac_present && *capacity < 100)
    {
        (*capacity)++;
        ups_set_status(1, 0, *capacity);
    }
    else
    {
        ups_set_status(ac_present, 0, *capacity);
    }
}

/* ============================================================
 * 系统时钟配置
 * ============================================================ */

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

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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

/* ============================================================
 * 错误处理
 * ============================================================ */

void Error_Handler(void)
{
    __disable_irq();
    while (1) {}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}
#endif
