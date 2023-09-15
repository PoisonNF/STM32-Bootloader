#include "ymodem.h"
#include "main.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"

/**
 * @brief 串口重定向（需要开启Use MicroLIB）
 * @param ch  		发送的数据
 * @param f         文件流
 * @return ch
 */
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

/**
 * @brief 发送指令
 * @param command  指令内容	
 * @return NULL
 */
void Send_Command(uint8_t command)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&command,1, 0xFFFF);
	HAL_Delay(10);
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
 * @brief 标记升级完成
 * @param NULL
 * @return NULL
 */
void Set_Update_Done(void)
{
	uint32_t update_flag = Startup_Update;				// 对应bootloader的启动步骤
	Flash_write((Application_2_Addr + Application_Size - 4), &update_flag,1 );
}

/* 临时存储的buff */
uint8_t save_buf[128] = {0};

#define POLY        0x1021  
/**
 * @brief CRC-16 校验
 * @param addr 开始地址
 * @param num   长度
 * @param crc   CRC
 * @return crc  返回CRC的值
 */
uint16_t crc16(uint8_t *addr, int num, uint16_t crc)  
{  
    int i;  
    for (; num > 0; num--)					/* Step through bytes in memory */  
    {  
        crc = crc ^ (*addr++ << 8);			/* Fetch byte from memory, XOR into CRC top byte*/  
        for (i = 0; i < 8; i++)				/* Prepare to rotate 8 bits */  
        {
            if (crc & 0x8000)				/* b15 is set... */  
                crc = (crc << 1) ^ POLY;  	/* rotate and XOR with polynomic */  
            else                          	/* b15 is clear... */  
                crc <<= 1;					/* just rotate */  
        }									/* Loop for 8 bits */  
        crc &= 0xFFFF;						/* Ensure CRC remains 16-bit value */  
    }										/* Loop until num=0 */  
    return(crc);							/* Return updated CRC */  
}

/**
 * @brief 获取数据包的类型, 顺便进行校验
 * @param buf 开始地址
 * @param len 长度
 * @return 1 通过校验 0 没通过校验
 */
uint8_t Check_CRC(uint8_t* buf, int len)
{
	uint16_t crc = 0;
	
	/* 进行CRC校验 */
	crc = crc16(buf+3, len - 5, crc);
	if(crc != (buf[131]<<8|buf[132]))	//buf[131]为CRCH，buf[132]为CRCL
	{
		printf("crc error\r\n");
		return 0;   /* 没通过校验 */
	}else{
		printf("crc ok\r\n");
    	return 1;	/* 通过校验 */
	}
}

static enum UPDATE_STATE update_state = TO_START;
/* 设置升级的步骤 */
static inline void Set_state(enum UPDATE_STATE state)
{
	update_state = state;
}


/* 查询升级的步骤 */
static inline uint8_t Get_state(void)
{
	return update_state;
}



uint8_t temp_buf[512] = {0};
uint8_t temp_len = 0;
/**
 * @brief 通过YModem协议升级
 * @param NULL
 * @return NULL
 */
void YModem_Update(void)
{
	if(Get_state()==TO_START)
	{
		Send_Command(CCC);
		HAL_Delay(1000);
	}
	if(Rx_Flag)    	// Receive flag
	{
		Rx_Flag = 0;	// clean flag
				
		/* 拷贝 */
		temp_len = Rx_Len;
        memcpy(temp_buf,Rx_Buf,temp_len);
		
		switch(temp_buf[0])
		{
			case SOH://数据包开始
			{
				static uint8_t data_state = 0;

				/* CRC16校验 */
				if(Check_CRC(temp_buf, temp_len))	
				{
					/* 起始帧 */			
					if((Get_state()==TO_START)&&(temp_buf[1] == 0x00)&&(temp_buf[2] == (uint8_t)(~temp_buf[1])))
					{
						printf("> Receive start...\r\n");

						Set_state(TO_RECEIVE_DATA);
						data_state = 0x01;						
						Send_Command(ACK);
						Send_Command(CCC);

						/* 擦除App2 */							
						Flash_Erase_page(Application_2_Addr,Application_Size/PAGESIZE);
						printf("> Erase APP2 %d page\r\n",Application_Size/PAGESIZE);
					}
					/* 结束帧 */
					else if((Get_state()==TO_RECEIVE_END)&&(temp_buf[1] == 0x00)&&(temp_buf[2] == (uint8_t)(~temp_buf[1])))
					{
						printf("> Receive end...\r\n");

						Set_Update_Done();		//标记升级完成					
						Set_state(TO_START);	//标记可以继续接收Ymodem数据	
						Send_Command(ACK);			
						HAL_NVIC_SystemReset();	//重启系统
					}
					/* 数据帧，对STX和SOH并没有区分 */					
					else if((Get_state()==TO_RECEIVE_DATA)&&(temp_buf[1] == data_state)&&(temp_buf[2] == (uint8_t)(~temp_buf[1])))
					{
						printf("> Receive data bag:%d byte\r\n",data_state * 128);
						
						/* 烧录程序 */
						Flash_write((Application_2_Addr + (data_state-1) * 128), (uint32_t *)(&temp_buf[3]), 32);
						data_state++;
						
						Send_Command(ACK);		
					}
				}
				else
				{
					printf("> Notpass crc\r\n");
				}
				
			}break;

			/* 结束传输 */
			case EOT:
			{
				if(Get_state()==TO_RECEIVE_DATA)
				{
					printf("> Receive EOT1...\r\n");
					
					Set_state(TO_RECEIVE_EOT2);					
					Send_Command(NACK);
				}
				else if(Get_state()==TO_RECEIVE_EOT2)
				{
					printf("> Receive EOT2...\r\n");
					
					Set_state(TO_RECEIVE_END);					
					Send_Command(ACK);
					Send_Command(CCC);
				}
				else
				{
					printf("> Receive EOT, But error...\r\n");
				}
			}break;	
		}
	}
}



