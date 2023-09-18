#ifndef __YMODEM_H
#define __YMODEM_H

/*=====用户配置(根据自己的分区进行配置)=====*/
#define BootLoader_Size 		0x10000U			// BootLoader的大小 40K
#define Application_Size		0xA000U		        // 应用程序的大小 40K

#define Application_1_Addr		0x08010000U		// 应用程序1的首地址
#define Application_2_Addr		0x0801A000U		// 应用程序2的首地址
/*==========================================*/

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

/* 启动的步骤 */
#define Startup_Normal 0xFFFFFFFF	// 正常启动
#define Startup_Update 0xAAAAAAAA	// 升级再启动
#define Startup_Reset  0x5555AAAA	// ***恢复出厂 目前没使用***

#define POLY        0x1021  

void YModem_Update(void);

#endif /* __YMODEM_H */
