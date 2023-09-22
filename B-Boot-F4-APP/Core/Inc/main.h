/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define VERSION "V0.1"	//�汾��

/*=====�û�����(�����Լ��ķ�����������)=====*/
#define BootLoader_Size 		0x20000U		// BootLoader�Ĵ�С 128K
#define Application_Size		0x60000U		// Ӧ�ó���Ĵ�С 384K

#define Application_1_Addr		0x08020000U		// Ӧ�ó���1���׵�ַ
#define Application_2_Addr		0x08080000U		// Ӧ�ó���2���׵�ַ
/*==========================================*/

/* �����Ĳ��� */
#define Startup_Normal 0xFFFFFFFF	// ��������
#define Startup_Update 0xAAAAAAAA	// ����������
#define Startup_Reset  0x5555AAAA	// ***�ָ����� Ŀǰûʹ��***

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */
extern UART_HandleTypeDef huart2;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
