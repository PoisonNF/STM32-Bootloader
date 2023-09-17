#include "ymodem.h"
#include "main.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"

uint8_t Rx_Flag;
uint16_t Rx_Len;
uint8_t Rx_Buf[Rx_Max];

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
	HAL_UART_Transmit(&huart2, (uint8_t *)&command,1, 0xFFFF);
	HAL_Delay(10);
}

/**
 * @brief ��ȡ��ַ���ڵ�sector
 * @param address  ��ʼ��ַ	
 * @return sector
 */
static uint32_t GetSector(uint32_t address)
{
    uint32_t sector = 0;

	(address <= 0x080FFFFF && address >= 0x080E0000)? sector = 11:
	(address >= 0x080C0000)? sector = 10:
	(address >= 0x080A0000)? sector = 9:
	(address >= 0x08080000)? sector = 8:
	(address >= 0x08060000)? sector = 7:
	(address >= 0x08040000)? sector = 6:
	(address >= 0x08020000)? sector = 5:
	(address >= 0x08010000)? sector = 4:
	(address >= 0x0800C000)? sector = 3:
	(address >= 0x08008000)? sector = 2:
	(address >= 0x08006000)? sector = 1:
	(address >= 0x08004000)? sector = 1:0;
	
    return sector;
}

/**
 * @brief flash����sector
 * @param start_addr  ��ʼ��ַ	
 * @param end_addr    ������ַ
 * @return 0 �ɹ� -1 ʧ��
 */
static int Flash_Erase_page(uint32_t start_addr, uint32_t end_addr)
{
	uint32_t UserStartSector;
	uint32_t SectorError = 0;
	FLASH_EraseInitTypeDef FlashSet;

	/* ����flash */
	HAL_FLASH_Unlock();
	
	/* ��ȡ��ʼ��ַ������������FLASH*/
	UserStartSector = GetSector(start_addr);

	FlashSet.TypeErase = TYPEERASE_SECTORS;
	FlashSet.Sector = UserStartSector;
	FlashSet.NbSectors = GetSector(end_addr) - UserStartSector;
	FlashSet.VoltageRange = VOLTAGE_RANGE_3;
    
	/*���ò�������*/
	HAL_FLASHEx_Erase(&FlashSet, &SectorError);
	
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
 * @brief ���App2�����������
 * @param NULL
 * @return NULL
 */
void Code_Storage_Done(void)
{
	uint32_t update_flag = Startup_Update;				// ��Ӧbootloader����������
	Flash_write((Application_2_Addr + Application_Size - 4), &update_flag,1 );   //��Bootloader����ӱ��
}

/* ��ʱ�洢��buff */
uint8_t save_buf[128] = {0};

#define POLY        0x1021  
/**
 * @brief CRC-16 У��
 * @param addr ��ʼ��ַ
 * @param num   ����
 * @param crc   CRC
 * @return crc  ����CRC��ֵ
 */
static uint16_t crc16(uint8_t *addr, int num, uint16_t crc)  
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
	crc = crc16(buf+3, len - 5, crc);
	if(crc != (buf[131]<<8|buf[132]))	//buf[131]ΪCRCH��buf[132]ΪCRCL
	{
		//printf("crc error\r\n");
		return 0;   /* ûͨ��У�� */
	}else{
		//printf("crc ok\r\n");
    	return 1;	/* ͨ��У�� */
	}
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
	if(Get_state()==TO_START)
	{
		Send_Command(CCC);
		HAL_Delay(1000);
	}
	if(Rx_Flag)    	// Receive flag
	{
		Rx_Flag = 0;	// clean flag
				
		/* ���� */
		temp_len = Rx_Len;
        memcpy(temp_buf,Rx_Buf,temp_len);
		
		switch(temp_buf[0])
		{
			case SOH://���ݰ���ʼ
			{
				static uint8_t data_state = 0;

				/* CRC16У�� */
				if(Check_CRC(temp_buf, temp_len))	
				{
					/* ��ʼ֡ */			
					if((Get_state()==TO_START)&&(temp_buf[1] == 0x00)&&(temp_buf[2] == (uint8_t)(~temp_buf[1])))
					{
						printf("> Receive start...\r\n");

						Set_state(TO_RECEIVE_DATA);
						data_state = 0x01;						
						Send_Command(ACK);
						Send_Command(CCC);

						/* ����App2 */							
						Flash_Erase_page(Application_2_Addr,Application_2_Addr + Application_Size - 1);
						printf("> Erase APP2 %d page\r\n",GetSector(Application_2_Addr + Application_Size - 1)-GetSector(Application_2_Addr) + 1);
					}
					/* ����֡ */
					else if((Get_state()==TO_RECEIVE_END)&&(temp_buf[1] == 0x00)&&(temp_buf[2] == (uint8_t)(~temp_buf[1])))
					{
						printf("> Receive end...\r\n");

						Code_Storage_Done();	//APP2�����������				
						Set_state(TO_START);	//��ǿ��Լ�������Ymodem����	
						Send_Command(ACK);			
						HAL_NVIC_SystemReset();	//����ϵͳ
					}
					/* ����֡����STX��SOH��û������ */					
					else if((Get_state()==TO_RECEIVE_DATA)&&(temp_buf[1] == data_state)&&(temp_buf[2] == (uint8_t)(~temp_buf[1])))
					{
						printf("> Receive data bag:%d byte\r\n",data_state * 128);
						
						/* ��¼���� */
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

			/* �������� */
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



