#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

void Error_Handler(void);
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_I2C1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_USART1_UART_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
