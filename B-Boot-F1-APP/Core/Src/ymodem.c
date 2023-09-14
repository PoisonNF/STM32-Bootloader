#include "ymodem.h"
#include "main.h"
#include "stdio.h"
#include "usart.h"

/**
 * @brief ����ָ��
 * @param command  ָ������	
 * @return NULL
 */
void send_command(uint8_t command)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&command,1, 0xFFFF);
	HAL_Delay(10);
}

/**
 * @brief flash����ҳ
 * @param pageaddr  ��ʼ��ַ	
 * @param num       ������ҳ��
 * @return 0 �ɹ� -1 ʧ��
 */
static int Flash_Erase_page(uint32_t pageaddr, uint32_t num)
{
	/* ����flash */
	HAL_FLASH_Unlock();
	
	/* ����FLASH*/
	FLASH_EraseInitTypeDef FlashSet;
	FlashSet.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashSet.PageAddress = pageaddr;
	FlashSet.NbPages = num;
	
	/*����PageError�����ò�������*/
	uint32_t PageError = 0;
	HAL_FLASHEx_Erase(&FlashSet, &PageError);
	
	/* ����flash */
	HAL_FLASH_Lock();
	return 0;
}

/**
 * @brief flashд���ɸ�����(word)
 * @param addr       д��ĵ�ַ
 * @param buf        д�����ݵ���ʼ��ַ
 * @param word_size  ����
 * @return NULL
 */
static void Flash_write(uint32_t addr,uint32_t *buf,uint32_t word_size)
{
	/* ����flash */
	HAL_FLASH_Unlock();

	/* ��д��ָ����ַ */
	for(int i = 0;i < word_size;i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,addr + 4*i,buf[i]);

	/* ����flash */
	HAL_FLASH_Lock();
    
    return;
}

/**
 * @brief ����������
 * @param NULL
 * @return NULL
 */
void Set_Update_Done(void)
{
	uint32_t update_flag = Startup_Update;				// ��Ӧbootloader����������
	Flash_write((Application_2_Addr + Application_Size - 4), &update_flag,1 );
}

/* ��ʱ�洢��buff */
uint8_t save_buf[128] = {0};

#define POLY        0x1021  
/**
 * @brief CRC-16 У��
 * @param addr ��ʼ��ַ
 * @param num   ����
 * @param num   CRC
 * @return crc  ����CRC��ֵ
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
 * @brief ��ȡ���ݰ�������, ˳�����У��
 * @param buf ��ʼ��ַ
 * @param len ����
 * @return 1 ͨ��У�� 0 ûͨ��У��
 */
uint8_t Check_CRC(uint8_t* buf, int len)
{
	unsigned short crc = 0;
	
	/* ����CRCУ�� */
	if((buf[0]==0x00)&&(len >= 133))
	{
		crc = crc16(buf+3, 128, crc);
		if(crc != (buf[131]<<8|buf[132]))
		{
			return 0;// ûͨ��У��
		}
		
		/* ͨ��У�� */
        return 1;
	}
    return 1;
}

static enum UPDATE_STATE update_state = TO_START;
/* ���������Ĳ��� */
static inline void Set_state(enum UPDATE_STATE state)
{
	update_state = state;
}


/* ��ѯ�����Ĳ��� */
static inline uint8_t Get_state(void)
{
	return update_state;
}



uint8_t temp_buf[512] = {0};
uint8_t temp_len = 0;
/**
 * @brief ͨ��YModemЭ������
 * @param NULL
 * @return NULL
 */
void YModem_Update(void)
{
	int i;
	if(Get_state()==TO_START)
	{
		send_command(CCC);
		HAL_Delay(1000);
	}
	if(Rx_Flag)    	// Receive flag
	{
		Rx_Flag=0;	// clean flag
				
		/* ���� */
		temp_len = Rx_Len;
		for(i = 0; i < temp_len; i++)
		{
			temp_buf[i] = Rx_Buf[i];
		}
		
		switch(temp_buf[0])
		{
			case SOH://���ݰ���ʼ
			{
				static uint8_t data_state = 0;
				if(Check_CRC(temp_buf, temp_len)==1)// ͨ��CRC16У��
				{					
					if((Get_state()==TO_START)&&(temp_buf[1] == 0x00)&&(temp_buf[2] == (uint8_t)(~temp_buf[1])))// ��ʼ
					{
						printf("> Receive start...\r\n");

						Set_state(TO_RECEIVE_DATA);
						data_state = 0x01;						
						send_command(ACK);
						send_command(CCC);

						/* ����App2 */							
						//Flash_Erase_page(Application_2_Addr, 40);
						Flash_Erase_page(Application_2_Addr,Application_Size/PAGESIZE);
						printf("> Erase APP2 %d page\r\n",Application_Size/PAGESIZE);
					}
					else if((Get_state()==TO_RECEIVE_END)&&(temp_buf[1] == 0x00)&&(temp_buf[2] == (uint8_t)(~temp_buf[1])))// ����
					{
						printf("> Receive end...\r\n");

						Set_Update_Done();						
						Set_state(TO_START);
						send_command(ACK);
						HAL_NVIC_SystemReset();
					}					
					else if((Get_state()==TO_RECEIVE_DATA)&&(temp_buf[1] == data_state)&&(temp_buf[2] == (uint8_t)(~temp_buf[1])))// ��������
					{
						printf("> Receive data bag:%d byte\r\n",data_state * 128);
						
						/* ��¼���� */
						Flash_write((Application_2_Addr + (data_state-1) * 128), (uint32_t *)(&temp_buf[3]), 32);
						data_state++;
						
						send_command(ACK);		
					}
				}
				else
				{
					printf("> Notpass crc\r\n");
				}
				
			}break;
			case EOT://���ݰ���ʼ
			{
				if(Get_state()==TO_RECEIVE_DATA)
				{
					printf("> Receive EOT1...\r\n");
					
					Set_state(TO_RECEIVE_EOT2);					
					send_command(NACK);
				}
				else if(Get_state()==TO_RECEIVE_EOT2)
				{
					printf("> Receive EOT2...\r\n");
					
					Set_state(TO_RECEIVE_END);					
					send_command(ACK);
					send_command(CCC);
				}
				else
				{
					printf("> Receive EOT, But error...\r\n");
				}
			}break;	
		}
	}
}


