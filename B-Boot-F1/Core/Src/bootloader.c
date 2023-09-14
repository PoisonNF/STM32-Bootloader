#include "bootloader.h"
#include "stdio.h"
#include "main.h"
#include "stdlib.h"
#include "string.h"
#include "usart.h"

/**
 * @brief 串口重定向（需要开启Use MicroLIB）
 * @param ch  		发送的数据	
 * @param f         文件流
 * @return ch
 */
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch,1, 0xFFFF);
    return ch;
}

/**
 * @brief flash擦除页
 * @param pageaddr  起始地址	
 * @param num       擦除的页数
 * @return 0 成功 -1 失败
 */
static int Flash_Erase_page(uint32_t pageaddr, uint32_t num)
{
	/* 解锁flash */
	HAL_FLASH_Unlock();
	
	/* 擦除FLASH*/
	FLASH_EraseInitTypeDef FlashSet;
	FlashSet.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashSet.PageAddress = pageaddr;
	FlashSet.NbPages = num;
	
	/*设置PageError，调用擦除函数*/
	uint32_t PageError = 0;
	HAL_FLASHEx_Erase(&FlashSet, &PageError);
	
	/* 锁定flash */
	HAL_FLASH_Lock();
	return 0;
}

/**
 * @brief flash写若干个数据(word)
 * @param addr       写入的地址
 * @param buf        写入数据的起始地址
 * @param word_size  长度
 * @return NULL
 */
static void Flash_write(uint32_t addr,uint32_t *buf,uint32_t word_size)
{
	/* 解锁flash */
	HAL_FLASH_Unlock();

	/* 烧写到指定地址 */
	for(int i = 0;i < word_size;i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,addr + 4*i,buf[i]);

	/* 锁定flash */
	HAL_FLASH_Lock();
    
    return;
}

/**
 * @brief flash读若干个数据(word)
 * @param addr       读数据的地址
 * @param buf        读出数据的数组指针
 * @param word_size  长度
 * @return 
 */
static inline void Flash_read(uint32_t addr, uint32_t *buf,uint32_t word_size)
{
	memcpy(buf, (uint32_t*) addr, word_size * sizeof(uint32_t));
}

/**
 * @brief 判断启动模式，读取APP2的最后一页
 * @param NULL
 * @return mode 用于选择启动模式
 */
uint32_t Read_Start_Mode(void)
{
	uint32_t mode = 0;

	/* 从APP2的最后一页读取一个word */
	Flash_read((Application_2_Addr + Application_Size - 4),&mode,1);

	return mode;
}

/**
 * @brief 进行程序的覆盖
 * @param  src_addr	搬运的源地址
 * @param  des_addr	搬运的目的地址
 * @param  byte_size 搬运的程序大小
 * @return NULL
 */
void MoveCode(uint32_t src_addr, uint32_t des_addr, uint32_t byte_size)
{
	/* 擦除目的地址 */
	printf("> Start erase des flash......\r\n");
	Flash_Erase_page(des_addr, (byte_size/PageSize));
	printf("> Erase des flash down......\r\n");
	
	/* 开始拷贝 */	
	uint32_t *temp;
	temp = (uint32_t *)calloc(0,256);
	
	printf("> Start copy......\r\n");
	for(int i = 0; i < byte_size/1024; i++)
	{
		Flash_read((src_addr + i*1024), temp, 256);
		Flash_write((des_addr + i*1024), temp, 256);
        memset(temp,0,256);
        printf("Copy %dKB now\r\n",i+1);
	}
	free(temp);
	printf("> Copy down......\r\n");
	
	/* 擦除源地址 */
	printf("> Start erase src flash......\r\n");
	Flash_Erase_page(src_addr, (byte_size/PageSize));
	printf("> Erase src flash down......\r\n");

}

/**
 * @brief 采用汇编设置栈的值
 * @param  ulAddr	地址
 * @return NULL
 */
__asm void MSR_MSP(uint32_t ulAddr) 
{
    MSR MSP, r0
    BX r14
}

typedef void (*Jump_Fun)(void);
/**
 * @brief 程序跳转函数
 * @param  App_Addr	应用程序地址
 * @return NULL
 */
void IAP_ExecuteApp(uint32_t App_Addr)
{
	Jump_Fun JumpToApp;

	/* 检查栈顶地址是否合法 */
	if (((*(__IO uint32_t *) App_Addr) & 0x2FFE0000 ) == 0x20000000)
	{
		JumpToApp = (Jump_Fun)*(__IO uint32_t *)(App_Addr + 4);	//用户代码区第二个字为程序开始地址(复位地址)
		MSR_MSP(*(__IO uint32_t *) App_Addr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)

		for(int i = 0;i < 8;i++)
		{
			NVIC->ICER[i] = 0xFFFFFFFF;	//关闭中断
			NVIC->ICPR[i] = 0xFFFFFFFF;	//清除中断标志位
		}

		/* 时钟复位 */
		HAL_RCC_DeInit();
		SysTick->CTRL = 0;
		SysTick->LOAD = 0;
		SysTick->VAL = 0;
		
		/* 跳转到APP */
		JumpToApp();	
	}
}

/**
 * @brief 进行BootLoader的启动
 * @param NULL
 * @return NULL
 */
void Start_BootLoader(void)
{
	/* Bootloader 信息打印 */
	printf("\r\n");
	printf("B-Boot %s %s\r\n",__DATE__,__TIME__);
	printf("Bootloader Version %s\r\n",B_BOOT_VERSION);
	printf("MCU: STM32F103VCT6 Running at 72MHz\r\n");
	printf("Bootloader Area from %#x - %#x\r\n",FLASH_BASE,Application_1_Addr - 1);
	printf("Application_1 Area from %#x - %#x\r\n",Application_1_Addr,Application_1_Addr + Application_Size -1);
	printf("Application_2 Area from %#x - %#x\r\n",Application_2_Addr,Application_2_Addr + Application_Size -1);
    printf("\r\n");

	printf("=> Choose a startup method......\r\n");	
	switch(Read_Start_Mode())										// 读取是否启动应用程序 */
	{
		case Startup_Normal:										// 正常启动 */
		{
			printf("=> Normal start......\r\n");
			break;
		}
		case Startup_Update:										// 升级再启动 */
		{
			printf("=> Start update......\r\n");		       
			MoveCode(Application_2_Addr, Application_1_Addr, Application_Size);
			printf("=> Update down......\r\n");
			break;
		}
		case Startup_Reset:											// 恢复出厂设置 目前没使用 */
		{
			printf("=> Restore to factory program......\r\n");
			break;
		}
		default:														// 启动失败
		{
			printf("=> Error:%X!!!......\r\n", Read_Start_Mode());
			return;
		}
	}

	/* 跳转到应用程序 */
	printf("=> Start Application_1......\r\n\r\n");	
	IAP_ExecuteApp(Application_1_Addr);
}

