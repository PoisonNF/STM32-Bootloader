#ifndef __YMODEM_H
#define __YMODEM_H

#include "stm32f1xx_hal.h"

/* YModem协议相关 */
#define SOH		0x01
#define STX		0x02
#define ACK		0x06
#define NACK	0x15
#define EOT		0x04
#define CCC		0x43

/* 升级的步骤 */
enum UPDATE_STATE
{
	TO_START = 0x01,
	TO_RECEIVE_DATA = 0x02,
	TO_RECEIVE_EOT1 = 0x03,
	TO_RECEIVE_EOT2 = 0x04,
	TO_RECEIVE_END = 0x05
};

void YModem_Update(void);

void Flash_Write(uint32_t addr,uint32_t *buf,uint32_t word_size);

void Flash_Read(uint32_t addr, uint32_t *buf,uint32_t word_size);

void Code_Storage_Done(void);

#endif /* __YMODEM_H */
