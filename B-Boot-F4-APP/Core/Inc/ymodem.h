#include "stm32f4xx_hal.h"

#ifndef __YMODEM_H
#define __YMODEM_H

/*=====�û�����(�����Լ��ķ�����������)=====*/
#define BootLoader_Size 		0x20000U		// BootLoader�Ĵ�С 128K
#define Application_Size		0x60000U		// Ӧ�ó���Ĵ�С 384K

#define Application_1_Addr		0x08020000U		// Ӧ�ó���1���׵�ַ
#define Application_2_Addr		0x08080000U		// Ӧ�ó���2���׵�ַ
/*==========================================*/

/* YModemЭ����� */
#define SOH		0x01
#define STX		0x02
#define ACK		0x06
#define NACK	0x15
#define EOT		0x04
#define CCC		0x43

/* �����Ĳ��� */
enum UPDATE_STATE
{
	TO_START = 0x01,
	TO_RECEIVE_DATA = 0x02,
	TO_RECEIVE_EOT1 = 0x03,
	TO_RECEIVE_EOT2 = 0x04,
	TO_RECEIVE_END = 0x05
};

/* �����Ĳ��� */
#define Startup_Normal 0xFFFFFFFF	// ��������
#define Startup_Update 0xAAAAAAAA	// ����������
#define Startup_Reset  0x5555AAAA	// ***�ָ����� Ŀǰûʹ��***

#define Rx_Max 1024  
extern uint8_t Rx_Flag;
extern uint16_t Rx_Len;
extern uint8_t Rx_Buf[Rx_Max];

void YModem_Update(void);

#endif /* __YMODEM_H */
