/* Phase 4 FreeRTOS entry point */
#include "main.h"
#include "phase4_freertos.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"
#include "dma.h"

static Phase4Runtime_t g_phase4_runtime;

void MX_GPIO_Init(void) {}
void MX_DMA_Init(void) {}
void MX_I2C1_Init(void) {}
void MX_USART1_UART_Init(void) {}
void MX_TIM2_Init(void) {}
void MX_TIM3_Init(void) {}
void SystemClock_Config(void) {}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_I2C1_Init();
    MX_TIM2_Init();
    MX_TIM3_Init();
    MX_USART1_UART_Init();

    OLED_Init();
    BT_Init();

    Phase4_InitRuntime(&g_phase4_runtime);
    Phase4_StartTasks(&g_phase4_runtime);

    vTaskStartScheduler();

    while (1) {
    }
}
