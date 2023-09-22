#include "main.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"
#include "dtu-4g.h"
#include "mqtt.h"
#include "ymodem.h"

/* 用于标记是否连接上服务器 */
static uint8_t Connect_Status = 0; 

/**
 * @brief DTU进入指令模式
 * 
 */
void DTU_Enter_CMD(void)
{
    uint8_t i = 5;

    u1_printf("进入指令模式...\r\n");
    while(1)
    {
        u3_printf("+++");
        u1_printf("剩余%ds...\r\n",i);
        HAL_Delay(1000);
        i--;

        /* 如果发送5次还是没应答则退出循环 */
        if(i == 0) 
        {
            u1_printf("进入指令模式失败!\r\n");
            break;
        }

        /* 等待DTU回应'a' */
        if(usart_info.ucDMARxCplt)
		{
			usart_info.ucDMARxCplt = 0;	//标志位清零
            if(usart_info.ucpDMARxCache[0] == 'a')
            {
                u1_printf("退出透传模式\r\n");
                u3_printf("a"); //回复DTU
                break;
            }
        }   
    }
}

/**
 * @brief DTU退出指令模式
 * 
 */
void DTU_Exit_CMD(void)
{
    u3_printf("AT+ENTM\r\n");
}

/**
 * @brief DTU向服务器发送数据
 * 
 * @param data 数据指针
 * @param datalen 数据长度
 * @return uint8_t 
 */
uint8_t DTU_SendData(uint8_t *data,uint16_t datalen)
{
    HAL_StatusTypeDef ret;
    ret = HAL_UART_Transmit(&DTU_USART,data,datalen,0xffff);
    if(ret == HAL_OK)   return 1;   //发送成功 
    else return 0;                  //发送失败
}

/**
 * @brief DTU设置远程服务器
 * 
 */
void DTU_Set_Server(void)
{
    u3_printf("AT+SOCKA=%s\r\n",SERVER_CONFIG);     //设置服务器地址和端口
    HAL_Delay(30);

    u3_printf("AT+SOCKAEN=ON\r\n");                 //开启SOCKA
    HAL_Delay(30);

    u3_printf("AT+SOCKBEN=OFF\r\n");                //关闭SOCKB
    HAL_Delay(30);

    u3_printf("AT+SOCKCEN=OFF\r\n");                //关闭SOCKC
    HAL_Delay(30);
    
    u3_printf("AT+SOCKDEN=OFF\r\n");                //关闭SOCKD
    HAL_Delay(30);

    u3_printf("AT+HEART=ON,NET,USER,60,C000\r\n");  //设置心跳
    HAL_Delay(30);

    u3_printf("AT+S\r\n");                          //保存配置
    HAL_Delay(30);
}

/**
 * @brief 处理DTU的串口得到的数据
 * 
 * @param data 数据指针
 * @param datalen 数据长度
 */
void DTU_Usart_Event(uint8_t *data,uint16_t datalen)
{
    //进入指令模式的应答
    if((datalen == 5) && !memcmp(data,"+ok\r\n",sizeof("+ok\r\n")))
    {
        u1_printf("进入指令模式，设置服务器...\r\n");

        //服务器配置
        DTU_Set_Server();

        //退出CMD模式
        DTU_Exit_CMD();
    }

    //退出指令模式的应答
    else if((datalen == 15) && !memcmp(data,"AT+ENTM\r\r\nOK\r\n",sizeof("AT+ENTM\r\r\nOK\r\n")))
    {
        u1_printf("退出指令模式!\r\n");
    }

    //配置服务器完成
    else if(!memcmp(data,"USR-DR152",sizeof("USR-DR152")))
    {
        u1_printf("Connect服务器...\r\n");
        MQTT_ConnectPack();
    }

    //接收到CONNACK报文
    else if((datalen == 4) && (data[0] == CONNACK))
    {
        u1_printf("收到CONNACK!\r\n");
        if(data[3] == 0x00){
            u1_printf("Connect服务器成功!\r\n");
            Connect_Status= 1;      //连接成功标志位
            MQTT_SubscribePack("/k08lcwgm0Ts/MQTTtest/user/get");
            HAL_Delay(100);
            MQTT_SubscribePack("/ota/device/upgrade/k08lcwgm0Ts/MQTTtest");
            HAL_Delay(100);
            MQTT_SubscribePack("/sys/k08lcwgm0Ts/MQTTtest/thing/file/download_reply");
            HAL_Delay(100);
            MQTT_SendOTAVersion();  //发送当前OTA的版本
        }else{
            u1_printf("Connect服务器失败!\r\n");
            NVIC_SystemReset();
        }
    }

    //接收到SUBACK报文
    else if(Connect_Status &&(datalen == 5) && (data[0] == SUBACK))
    {
        u1_printf("收到SUBACK!\r\n");

        //如果最后一个字节为0x00或者0x01代表发送成功
        if((data[datalen-1] == 0x00) || (data[datalen-1] == 0x01)){
            u1_printf("Subscribe生效!\r\n");
            //MQTT_UnSubscribePack("/k08lcwgm0Ts/MQTTtest/user/get");
        }else{
            u1_printf("Subscribe错误!\r\n");
            NVIC_SystemReset();
        }
    }

    //接收到UNSUBACK报文
    else if(Connect_Status && (data[0] == UNSUBACK) && data[1] == 0x02)
    {
        u1_printf("收到UNSUBACK!\r\n");
        u1_printf("UnSubscribe生效!\r\n");
    }

    //接收到等级0的Publish报文
    else if(Connect_Status && (data[0] == PUBLISHQOS0))
    {
        u1_printf("收到等级0的Publish报文!\r\n");
        MQTT_DealPublishData(data,datalen);
        if(strstr((const char*)Aliyun_mqtt.CMD_buff,"{\"switch1\":0}"))
        {
            u1_printf("关闭开关\r\n");
            MQTT_PublishDataQos0("/k08lcwgm0Ts/MQTTtest/user/post","{\"params\":}{\"switch1\":0}");
        }
        if(strstr((const char*)Aliyun_mqtt.CMD_buff,"{\"switch1\":1}"))
        {
            u1_printf("打开开关\r\n");
            MQTT_PublishDataQos0("/k08lcwgm0Ts/MQTTtest/user/post","{\"params\":}{\"switch1\":1}");
        }

        if(strstr((const char*)Aliyun_mqtt.CMD_buff,"upgrade"))
            MQTT_GetOTAInfo((char*)Aliyun_mqtt.CMD_buff);

        //收到download_reply,为bin文件的分片
        if(strstr((const char*)Aliyun_mqtt.CMD_buff,"/sys/k08lcwgm0Ts/MQTTtest/thing/file/download_reply"))
        {
            u1_printf("一共%d字节\r\n",datalen);
            for(int i = 0;i < datalen;i++)
                u1_printf("%02x ",data[i]);
            u1_printf("\r\n");

            u1_printf("第%d字节处存放 %02x\r\n",(Aliyun_mqtt.num-1) * 256 + datalen - Aliyun_mqtt.downlen -2,data[datalen - Aliyun_mqtt.downlen -2]);
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
                u1_printf("OTA下载完毕!\r\n");
                Code_Storage_Done();
                NVIC_SystemReset();
            }
        }
    }

    //接收到PUBACK报文（QoS1）
    else if(Connect_Status && (data[0] == PUBACK))
    {
        u1_printf("收到PUBACK!\r\n");
    }

    //接收到d0 00 代表设备保活
    else if(Connect_Status && (datalen == 2) && (data[0] == PINGRESP) && (data[1] == 0x00))
    {
        u1_printf("设备存活!\r\n");
    }
}

/**
 * @brief DTU工作函数
 * 
 */
void DTU_Working(void)
{
    uint8_t DataBuf[512];
    uint16_t DataLen;

    /* 如果4g模块的串口接收到数据 */
	if(usart_info.ucDMARxCplt)
	{
		usart_info.ucDMARxCplt = 0;	//标志位清零

		//数据拷贝
		memcpy(DataBuf,usart_info.ucpDMARxCache,usart_info.usDMARxLength);
		DataLen = usart_info.usDMARxLength;

		// for(int i = 0;i<DataLen;i++)
		//  	u1_printf("%02x ",DataBuf[i]);
		//  u1_printf("\r\n");
		//u1_printf("%s",DataBuf);
		//u1_printf("%d\r\n",DataLen);

		//处理DTU数据
		DTU_Usart_Event(DataBuf,DataLen);
	}
}

