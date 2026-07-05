/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* TIM2 init function - 1Hz update interrupt for RTC */
void MX_TIM2_Init(void)
{
  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  /* TIM2 clock enable */
  __HAL_RCC_TIM2_CLK_ENABLE();

  /* Timer configuration for 1Hz tick
     APB1 clock is 36MHz, but TIM2 input clock is 72MHz (APB1 pre-scaler = 2)
     PSC = 7199  -> 72MHz / 7200 = 10kHz
     ARR = 9999  -> 10kHz / 10000 = 1Hz */
  TIM2->PSC  = 7199U;
  TIM2->ARR  = 9999U;

  /* Counter mode: up, update request source: counter overflow only */
  TIM2->CR1 = TIM_CR1_URS;

  /* Auto-reload preload disabled (CR1 ARPE bit = 0, already reset) */

  /* Generate an update event to reload Prescaler and Auto-reload value */
  TIM2->EGR = TIM_EGR_UG;

  /* Clear the update interrupt flag that was set by UG */
  TIM2->SR = ~TIM_SR_UIF;

  /* Enable the update interrupt */
  TIM2->DIER = TIM_DIER_UIE;

  /* TIM2 interrupt enable in NVIC */
  HAL_NVIC_SetPriority(TIM2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);

  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
}

void MX_TIM2_Start(void)
{
  /* Enable the counter */
  TIM2->CR1 |= TIM_CR1_CEN;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */