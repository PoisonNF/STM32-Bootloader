#ifndef __YMODEM_H
#define __YMODEM_H

/*=====�û�����(�����Լ��ķ�����������)=====*/
#define BootLoader_Size 		0x9000U		    // BootLoader�Ĵ�С 36K
#define Application_Size		0x1B000U	    // Ӧ�ó���Ĵ�С 110K

#define Application_1_Addr		0x08009000U		// Ӧ�ó���1���׵�ַ
#define Application_2_Addr		0x08024000U		// Ӧ�ó���2���׵�ַ
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

#define POLY        0x1021  

void YModem_Update(void);

#endif /* __YMODEM_H */
