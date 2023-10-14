#ifndef __BOOTLOADER_H_
#define __BOOTLOADER_H_

/* bootloader版本号 */
#define B_BOOT_VERSION  "0.2"
/* 设备序号 */
#define DEVICE_NO        1      //根据设备名进行更改，定死写在bootloader中

/*=====用户配置(根据自己的分区进行配置)=====*/
#define BootLoader_Size 		0x20000U		// BootLoader的大小 128K
#define Application_Size		0x60000U		// 应用程序的大小 384K

#define Application_1_Addr		0x08020000U		// 应用程序1的首地址
#define Application_2_Addr		0x08080000U		// 应用程序2的首地址
/*==========================================*/

/* 启动的步骤 */
#define Startup_Normal 0xFFFFFFFF	// 正常启动
#define Startup_Update 0xAAAAAAAA	// 升级再启动
#define Startup_Reset  0x5555AAAA	// ***恢复出厂 目前没使用***

void Start_BootLoader(void);

#endif // !__BOOTLER_H_

