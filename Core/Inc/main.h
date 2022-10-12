/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_1
#define B1_GPIO_Port GPIOA
#define B1_EXTI_IRQn EXTI0_1_IRQn
#define NixieDigitA_Pin GPIO_PIN_2
#define NixieDigitA_GPIO_Port GPIOA
#define NixieDigitB_Pin GPIO_PIN_3
#define NixieDigitB_GPIO_Port GPIOA
#define NixieDigitC_Pin GPIO_PIN_4
#define NixieDigitC_GPIO_Port GPIOA
#define NixieDigitD_Pin GPIO_PIN_5
#define NixieDigitD_GPIO_Port GPIOA
#define TIM16_ALARM_NOISE_Pin GPIO_PIN_6
#define TIM16_ALARM_NOISE_GPIO_Port GPIOA
#define B0_Pin GPIO_PIN_0
#define B0_GPIO_Port GPIOB
#define B0_EXTI_IRQn EXTI0_1_IRQn
#define Nixie_1_Pin GPIO_PIN_8
#define Nixie_1_GPIO_Port GPIOA
#define Nixie_2_Pin GPIO_PIN_9
#define Nixie_2_GPIO_Port GPIOA
#define Nixie_3_Pin GPIO_PIN_10
#define Nixie_3_GPIO_Port GPIOA
#define Nixie_4_Pin GPIO_PIN_11
#define Nixie_4_GPIO_Port GPIOA
#define MoveSensor_Pin GPIO_PIN_12
#define MoveSensor_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

typedef enum {
	All,
	Nixie1,
	Nixie2,
	Nixie3,
	Nixie4,
	None
} LightUpTube ;

typedef enum {
	Time,
	Alarm,
	TimeSet,
	AlarmSet
} ClkState;

typedef enum {
	ContinuouseMode,
	MoveMode
} Mode;

typedef enum {
	NotPressed,
	SingleClick,
	DoubleClick,
	Hold
} ButtonStates;

typedef enum {
	ON,
	OFF
} AlarmState;


typedef struct {
	volatile bool flag;
	uint32_t timer;
	uint8_t buttonTimerEnable;
	ButtonStates buttonState;
	uint8_t	buttonHoldCounter;
}MultiButton;

typedef struct {
	uint8_t Hours;
	uint8_t Minutes;
} TimeHolder;

typedef struct {
	volatile Mode DispMode;
	volatile ClkState ClockState;
	volatile AlarmState Alarm;
	volatile LightUpTube Tube;
	volatile LightUpTube NextNixie;
	TimeHolder ClockTime;
	TimeHolder AlarmTime;
} Clk;

typedef struct {
	uint8_t HoursDecimal;
	uint8_t HoursUnity;
	uint8_t MinutesDecimal;
	uint8_t MinutesUnity;
}DisplayTime;
/*
 * Structure holding hour and minutes value read from the rtc as decimal value
 */


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
