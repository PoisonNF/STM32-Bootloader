#ifndef __DTU_4G_H_
#define __DTU_4G_H_

#include "stdint.h"

/* DTUʹ�õĴ��� */
#define DTU_USART huart3

/* Զ�̷��������� */
#define SERVER_CONFIG	"TCP,iot-060a70tq.mqtt.iothub.aliyuncs.com,1883"

void DTU_Enter_CMD(void);

void DTU_Exit_CMD(void);

uint8_t DTU_SendData(uint8_t *data, uint16_t datalen);

void DTU_Set_Server(void);

void DTU_Usart_Event(uint8_t *data, uint16_t datalen);

void DTU_SendOTAVersion(void);

void DTU_GetOTAInfo(char *data);

void DTU_OTA_Download(int size, int offset);

void DTU_Working(void);

#endif // !__DTU-4G_H_

