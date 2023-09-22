#include "ymodem.h"
#include "main.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"

/**
 * @brief �����ض�����Ҫ����Use MicroLIB��
 * @param ch  		���͵�����
 * @param f         �ļ���
 * @return ch
 */
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}

/**
 * @brief ����ָ��
 * @param command  ָ������	
 * @return NULL
 */
static void Send_Command(uint8_t command)
{
	HAL_UART_Transmit(&huart3, (uint8_t *)&command,1, 0xFFFF);
	HAL_Delay(10);
}

/**
 * @brief flash����ҳ
 * @param pageaddr  ��ʼ��ַ	
 * @param num       ������ҳ��
 * @return 0 �ɹ� -1 ʧ��
 */
static int Flash_Erase_Page(uint32_t pageaddr, uint32_t num)
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
void Flash_Write(uint32_t addr,uint32_t *buf,uint32_t word_size)
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
 * @brief ���App2�����������
 * @param NULL
 * @return NULL
 */
void Code_Storage_Done(void)
{
	uint32_t update_flag = Startup_Update;				// ��Ӧbootloader����������
	Flash_Write((Application_2_Addr + Application_Size - 4), &update_flag,1 );   //��Bootloader�����ӱ��
}


#define POLY        0x1021  
/**
 * @brief CRC-16 У��
 * @param addr ��ʼ��ַ
 * @param num   ����
 * @param crc   CRC
 * @return crc  ����CRC��ֵ
 */
static uint16_t CRC16(uint8_t *addr, int num, uint16_t crc)  
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
	uint16_t crc = 0;
	
	/* ����CRCУ�� */
	crc = CRC16(buf+3, len - 5, crc);
	if(crc != (buf[131]<<8|buf[132]))	//buf[131]ΪCRCH��buf[132]ΪCRCL
	{
		//printf("crc error\r\n");
		return 0;   /* ûͨ��У�� */
	}else{
		//printf("crc ok\r\n");
    	return 1;	/* ͨ��У�� */
	}
}

static enum UPDATE_STATE Update_State = TO_START;
/* ���������Ĳ��� */
static inline void Set_State(enum UPDATE_STATE state)
{
	Update_State = state;
}

/* ��ѯ�����Ĳ��� */
static inline uint8_t Get_State(void)
{
	return Update_State;
}

uint8_t Temp_Buf[512] = {0};
uint8_t Temp_Len = 0;
/**
 * @brief ͨ��YModemЭ������
 * @param NULL
 * @return NULL
 */
void YModem_Update(void)
{
	if(Get_State()==TO_START)
	{
		Send_Command(CCC);
		HAL_Delay(1000);
	}
	if(usart_info.ucDMARxCplt)    	// Receive flag
	{
		usart_info.ucDMARxCplt = 0;	// clean flag
				
		/* ���� */
		Temp_Len = usart_info.usDMARxLength;
        memcpy(Temp_Buf,usart_info.ucpDMARxCache,Temp_Len);
		
		switch(Temp_Buf[0])
		{
			case SOH://���ݰ���ʼ
			{
				static uint8_t Data_State = 0;

				/* CRC16У�� */
				if(Check_CRC(Temp_Buf, Temp_Len))	
				{
					/* ��ʼ֡ */			
					if((Get_State()==TO_START)&&(Temp_Buf[1] == 0x00)&&(Temp_Buf[2] == (uint8_t)(~Temp_Buf[1])))
					{
						printf("> Receive start...\r\n");

						Set_State(TO_RECEIVE_DATA);
						Data_State = 0x01;						
						Send_Command(ACK);
						Send_Command(CCC);

						/* ����App2 */							
						Flash_Erase_Page(Application_2_Addr,Application_Size/PAGESIZE);
						printf("> Erase APP2 %d page\r\n",Application_Size/PAGESIZE);
					}
					/* ����֡ */
					else if((Get_State()==TO_RECEIVE_END)&&(Temp_Buf[1] == 0x00)&&(Temp_Buf[2] == (uint8_t)(~Temp_Buf[1])))
					{
						printf("> Receive end...\r\n");

						Code_Storage_Done();	//APP2�����������				
						Set_State(TO_START);	//��ǿ��Լ�������Ymodem����	
						Send_Command(ACK);			
						HAL_NVIC_SystemReset();	//����ϵͳ
					}
					/* ����֡����STX��SOH��û������ */					
					else if((Get_State()==TO_RECEIVE_DATA)&&(Temp_Buf[1] == Data_State)&&(Temp_Buf[2] == (uint8_t)(~Temp_Buf[1])))
					{
						printf("> Receive data bag:%d byte\r\n",Data_State * 128);
						
						/* ��¼���� */
						Flash_Write((Application_2_Addr + (Data_State-1) * 128), (uint32_t *)(&Temp_Buf[3]), 32);
						Data_State++;
						
						Send_Command(ACK);		
					}
				}
				else
				{
					printf("> Notpass crc\r\n");
				}
				
			}break;

			/* �������� */
			case EOT:
			{
				if(Get_State()==TO_RECEIVE_DATA)
				{
					printf("> Receive EOT1...\r\n");
					
					Set_State(TO_RECEIVE_EOT2);					
					Send_Command(NACK);
				}
				else if(Get_State()==TO_RECEIVE_EOT2)
				{
					printf("> Receive EOT2...\r\n");
					
					Set_State(TO_RECEIVE_END);					
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


