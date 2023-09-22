#include "main.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"
#include "dtu-4g.h"
#include "mqtt.h"
#include "ymodem.h"

/* ���ڱ���Ƿ������Ϸ����� */
static uint8_t Connect_Status = 0; 

/**
 * @brief DTU����ָ��ģʽ
 * 
 */
void DTU_Enter_CMD(void)
{
    uint8_t i = 5;

    u1_printf("����ָ��ģʽ...\r\n");
    while(1)
    {
        u3_printf("+++");
        u1_printf("ʣ��%ds...\r\n",i);
        HAL_Delay(1000);
        i--;

        /* �������5�λ���ûӦ�����˳�ѭ�� */
        if(i == 0) 
        {
            u1_printf("����ָ��ģʽʧ��!\r\n");
            break;
        }

        /* �ȴ�DTU��Ӧ'a' */
        if(usart_info.ucDMARxCplt)
		{
			usart_info.ucDMARxCplt = 0;	//��־λ����
            if(usart_info.ucpDMARxCache[0] == 'a')
            {
                u1_printf("�˳�͸��ģʽ\r\n");
                u3_printf("a"); //�ظ�DTU
                break;
            }
        }   
    }
}

/**
 * @brief DTU�˳�ָ��ģʽ
 * 
 */
void DTU_Exit_CMD(void)
{
    u3_printf("AT+ENTM\r\n");
}

/**
 * @brief DTU���������������
 * 
 * @param data ����ָ��
 * @param datalen ���ݳ���
 * @return uint8_t 
 */
uint8_t DTU_SendData(uint8_t *data,uint16_t datalen)
{
    HAL_StatusTypeDef ret;
    ret = HAL_UART_Transmit(&DTU_USART,data,datalen,0xffff);
    if(ret == HAL_OK)   return 1;   //���ͳɹ� 
    else return 0;                  //����ʧ��
}

/**
 * @brief DTU����Զ�̷�����
 * 
 */
void DTU_Set_Server(void)
{
    u3_printf("AT+SOCKA=%s\r\n",SERVER_CONFIG);     //���÷�������ַ�Ͷ˿�
    HAL_Delay(30);

    u3_printf("AT+SOCKAEN=ON\r\n");                 //����SOCKA
    HAL_Delay(30);

    u3_printf("AT+SOCKBEN=OFF\r\n");                //�ر�SOCKB
    HAL_Delay(30);

    u3_printf("AT+SOCKCEN=OFF\r\n");                //�ر�SOCKC
    HAL_Delay(30);
    
    u3_printf("AT+SOCKDEN=OFF\r\n");                //�ر�SOCKD
    HAL_Delay(30);

    u3_printf("AT+HEART=ON,NET,USER,60,C000\r\n");  //��������
    HAL_Delay(30);

    u3_printf("AT+S\r\n");                          //��������
    HAL_Delay(30);
}

/**
 * @brief ����DTU�Ĵ��ڵõ�������
 * 
 * @param data ����ָ��
 * @param datalen ���ݳ���
 */
void DTU_Usart_Event(uint8_t *data,uint16_t datalen)
{
    //����ָ��ģʽ��Ӧ��
    if((datalen == 5) && !memcmp(data,"+ok\r\n",sizeof("+ok\r\n")))
    {
        u1_printf("����ָ��ģʽ�����÷�����...\r\n");

        //����������
        DTU_Set_Server();

        //�˳�CMDģʽ
        DTU_Exit_CMD();
    }

    //�˳�ָ��ģʽ��Ӧ��
    else if((datalen == 15) && !memcmp(data,"AT+ENTM\r\r\nOK\r\n",sizeof("AT+ENTM\r\r\nOK\r\n")))
    {
        u1_printf("�˳�ָ��ģʽ!\r\n");
    }

    //���÷��������
    else if(!memcmp(data,"USR-DR152",sizeof("USR-DR152")))
    {
        u1_printf("Connect������...\r\n");
        MQTT_ConnectPack();
    }

    //���յ�CONNACK����
    else if((datalen == 4) && (data[0] == CONNACK))
    {
        u1_printf("�յ�CONNACK!\r\n");
        if(data[3] == 0x00){
            u1_printf("Connect�������ɹ�!\r\n");
            Connect_Status= 1;      //���ӳɹ���־λ
            MQTT_SubscribePack("/k08lcwgm0Ts/MQTTtest/user/get");
            HAL_Delay(100);
            MQTT_SubscribePack("/ota/device/upgrade/k08lcwgm0Ts/MQTTtest");
            HAL_Delay(100);
            MQTT_SubscribePack("/sys/k08lcwgm0Ts/MQTTtest/thing/file/download_reply");
            HAL_Delay(100);
            MQTT_SendOTAVersion();  //���͵�ǰOTA�İ汾
        }else{
            u1_printf("Connect������ʧ��!\r\n");
            NVIC_SystemReset();
        }
    }

    //���յ�SUBACK����
    else if(Connect_Status &&(datalen == 5) && (data[0] == SUBACK))
    {
        u1_printf("�յ�SUBACK!\r\n");

        //������һ���ֽ�Ϊ0x00����0x01�����ͳɹ�
        if((data[datalen-1] == 0x00) || (data[datalen-1] == 0x01)){
            u1_printf("Subscribe��Ч!\r\n");
            //MQTT_UnSubscribePack("/k08lcwgm0Ts/MQTTtest/user/get");
        }else{
            u1_printf("Subscribe����!\r\n");
            NVIC_SystemReset();
        }
    }

    //���յ�UNSUBACK����
    else if(Connect_Status && (data[0] == UNSUBACK) && data[1] == 0x02)
    {
        u1_printf("�յ�UNSUBACK!\r\n");
        u1_printf("UnSubscribe��Ч!\r\n");
    }

    //���յ��ȼ�0��Publish����
    else if(Connect_Status && (data[0] == PUBLISHQOS0))
    {
        u1_printf("�յ��ȼ�0��Publish����!\r\n");
        MQTT_DealPublishData(data,datalen);
        if(strstr((const char*)Aliyun_mqtt.CMD_buff,"{\"switch1\":0}"))
        {
            u1_printf("�رտ���\r\n");
            MQTT_PublishDataQos0("/k08lcwgm0Ts/MQTTtest/user/post","{\"params\":}{\"switch1\":0}");
        }
        if(strstr((const char*)Aliyun_mqtt.CMD_buff,"{\"switch1\":1}"))
        {
            u1_printf("�򿪿���\r\n");
            MQTT_PublishDataQos0("/k08lcwgm0Ts/MQTTtest/user/post","{\"params\":}{\"switch1\":1}");
        }

        if(strstr((const char*)Aliyun_mqtt.CMD_buff,"upgrade"))
            MQTT_GetOTAInfo((char*)Aliyun_mqtt.CMD_buff);

        //�յ�download_reply,Ϊbin�ļ��ķ�Ƭ
        if(strstr((const char*)Aliyun_mqtt.CMD_buff,"/sys/k08lcwgm0Ts/MQTTtest/thing/file/download_reply"))
        {
            u1_printf("һ��%d�ֽ�\r\n",datalen);
            for(int i = 0;i < datalen;i++)
                u1_printf("%02x ",data[i]);
            u1_printf("\r\n");

            u1_printf("��%d�ֽڴ���� %02x\r\n",(Aliyun_mqtt.num-1) * 256 + datalen - Aliyun_mqtt.downlen -2,data[datalen - Aliyun_mqtt.downlen -2]);
            Flash_Write(Application_2_Addr + (Aliyun_mqtt.num-1) * 256,(uint32_t *)&data[datalen - Aliyun_mqtt.downlen -2],64);
            Aliyun_mqtt.num++;
            if(Aliyun_mqtt.num < Aliyun_mqtt.counter)
            {
                Aliyun_mqtt.downlen = 256;
                MQTT_OTA_Download(Aliyun_mqtt.downlen,(Aliyun_mqtt.num-1) * 256);
            }
            else if(Aliyun_mqtt.num == Aliyun_mqtt.counter)
            {
                if(Aliyun_mqtt.size % 256 == 0)
                {
                    Aliyun_mqtt.downlen = 256;
                    MQTT_OTA_Download(Aliyun_mqtt.downlen,(Aliyun_mqtt.num-1) * 256);
                }
                else
                {
                    Aliyun_mqtt.downlen = Aliyun_mqtt.size % 256;
                    MQTT_OTA_Download(Aliyun_mqtt.downlen,(Aliyun_mqtt.num-1) * 256);
                }
            }
            else
            {
                u1_printf("OTA�������!\r\n");
                Code_Storage_Done();
                NVIC_SystemReset();
            }
        }
    }

    //���յ�PUBACK���ģ�QoS1��
    else if(Connect_Status && (data[0] == PUBACK))
    {
        u1_printf("�յ�PUBACK!\r\n");
    }

    //���յ�d0 00 �����豸����
    else if(Connect_Status && (datalen == 2) && (data[0] == PINGRESP) && (data[1] == 0x00))
    {
        u1_printf("�豸���!\r\n");
    }
}

/**
 * @brief DTU��������
 * 
 */
void DTU_Working(void)
{
    uint8_t DataBuf[512];
    uint16_t DataLen;

    /* ���4gģ��Ĵ��ڽ��յ����� */
	if(usart_info.ucDMARxCplt)
	{
		usart_info.ucDMARxCplt = 0;	//��־λ����

		//���ݿ���
		memcpy(DataBuf,usart_info.ucpDMARxCache,usart_info.usDMARxLength);
		DataLen = usart_info.usDMARxLength;

		// for(int i = 0;i<DataLen;i++)
		//  	u1_printf("%02x ",DataBuf[i]);
		//  u1_printf("\r\n");
		//u1_printf("%s",DataBuf);
		//u1_printf("%d\r\n",DataLen);

		//����DTU����
		DTU_Usart_Event(DataBuf,DataLen);
	}
}

