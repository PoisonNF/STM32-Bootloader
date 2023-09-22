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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define VERSION "V0.1"	//�汾��

/*=====�û�����(�����Լ��ķ�����������)=====*/
#define BootLoader_Size 		0x9000U		    // BootLoader�Ĵ�С 36K
#define Application_Size		0x1B000U	    // Ӧ�ó���Ĵ�С 108K

#define Application_1_Addr		0x08009000U		// Ӧ�ó���1���׵�ַ
#define Application_2_Addr		0x08024000U		// Ӧ�ó���2���׵�ַ
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
#define Rx_Max 1024  	 
extern uint8_t Rx_Flag;
extern uint16_t	Rx_Len;
extern uint8_t Rx_Buf[Rx_Max];
extern UART_HandleTypeDef huart2;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
