/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */
//串口1重定向
static uint8_t USART1_TX_BUF[200];
#define u1_printf(...)  HAL_UART_Transmit(&huart1,USART1_TX_BUF,sprintf((char *)USART1_TX_BUF,__VA_ARGS__),0xffff)
//串口3重定向                                                                                                                                                                  
static uint8_t USART3_TX_BUF[200];
#define u3_printf(...)  HAL_UART_Transmit(&huart3,USART3_TX_BUF,sprintf((char *)USART3_TX_BUF,__VA_ARGS__),0xffff)

/* DTU串口信息结构体 */
typedef struct {

  /* DMA相关信息 */
	uint16_t			usDMARxLength;			/* DMA总接收数据长度(DMA使用) */
	uint16_t			usDMARxMAXSize;			/* DMA接收缓冲区大小(DMA使用) */
	uint8_t				*ucpDMARxCache;			/* DMA接收缓冲区(DMA使用) */

	/* 标志位信息 */
	uint8_t				ucDMARxCplt;			/* DMA接收完成标志(DMA使用) */
}USART_INFO_T;

extern USART_INFO_T usart_info;

/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

