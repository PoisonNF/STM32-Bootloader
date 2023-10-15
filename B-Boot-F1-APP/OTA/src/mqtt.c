#include "main.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"
#include "mqtt.h"
#include "dtu-4g.h"
#include "sort.h"

/* MQTT控制块 */
MQTT_CB Aliyun_mqtt;

extern struct DeviceInfo Dev;

/**
 * @brief MQTT剩余长度字节数判断处理
 * 
 */
static void MQTT_Remaining_Len_Process(void)
{
    //判断剩余长度是一个字节还是两个字节
    do{
        if(Aliyun_mqtt.Remaining_len/128 == 0)
            Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = Aliyun_mqtt.Remaining_len;
        else
            Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len] = (Aliyun_mqtt.Remaining_len%128) | 0x80;

        Aliyun_mqtt.Fixed_len++;
        Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Remaining_len/128;
    }while(Aliyun_mqtt.Remaining_len);
}

/**
 * @brief MQTT的Connect报文
 * 
 */
void MQTT_ConnectPack(void)
{
    Aliyun_mqtt.MessageID = 1;
    Aliyun_mqtt.Fixed_len = 1;
    Aliyun_mqtt.Variable_len = 10;
    Aliyun_mqtt.Payload_len = 2 + strlen(Dev.clientId) + 2 + strlen(Dev.username) + 2 + strlen(Dev.passwd);   //其中的2都表示为长度
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;

    /* 固定报头 */
    Aliyun_mqtt.Pack_buff[0] = CONNECT;    //Connect报头

    //判断剩余长度是一个字节还是两个字节
    MQTT_Remaining_Len_Process();

    /* 可变报头 */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = 0x00;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = 0x04;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2] = 0x4D;    //'M'
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3] = 0x51;    //'Q'
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4] = 0x54;    //'T'
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 5] = 0x54;    //'T'
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 6] = 0x04;    //协议级别04
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 7] = 0xC2;    //连接标志
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 8] = 0x00;    //保持连接时间高字节（100）
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 9] = 0x64;    //保持连接时间低字节（100）

    /* 有效负载 */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 10] = strlen(Dev.clientId)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 11] = strlen(Dev.clientId)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 12],Dev.clientId,strlen(Dev.clientId));

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 12 + strlen(Dev.clientId)] = strlen(Dev.username)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 13 + strlen(Dev.clientId)] = strlen(Dev.username)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 14 + strlen(Dev.clientId)],Dev.username,strlen(Dev.username));

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 14 + strlen(Dev.clientId) + strlen(Dev.username)] = strlen(Dev.passwd)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 15 + strlen(Dev.clientId) + strlen(Dev.username)] = strlen(Dev.passwd)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 16 + strlen(Dev.clientId) + strlen(Dev.username)],Dev.passwd,strlen(Dev.passwd));

    //测试输出数据
    // for(int i = 0;i < (Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len);i++)
    //     u1_printf("%02x ",Aliyun_mqtt.Pack_buff[i]);
    //HAL_UART_Transmit(&DTU_USART,Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len,0xffff);

    /* 使用DTU发送Connect报文给服务器 */
    if(DTU_SendData(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len))
        u1_printf("Connect报文发送成功,等待服务器回应\r\n");

}

/**
 * @brief MQTT的Subscribe报文
 * 
 */
void MQTT_SubscribePack(char *topic)
{
    Aliyun_mqtt.Fixed_len = 1;
    Aliyun_mqtt.Variable_len = 2;
    Aliyun_mqtt.Payload_len = 2 + strlen(topic) + 1;   //其中的2都表示为长度,1代表服务质量等级
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;

    /* 固定报头 */
    Aliyun_mqtt.Pack_buff[0] = SUBSCRIBE;    //Subscrib报头

    //判断剩余长度是一个字节还是两个字节
    MQTT_Remaining_Len_Process();

    /* 可变报头 */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = Aliyun_mqtt.MessageID/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = Aliyun_mqtt.MessageID%256;
    Aliyun_mqtt.MessageID++;

    /* 有效负载 */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2] = strlen(topic)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3] = strlen(topic)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4],topic,strlen(topic));

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4 + strlen(topic)] = 0;   //服务质量等级为0

    if(DTU_SendData(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len))
        u1_printf("Subscribe报文发送成功,等待服务器回应\r\n");
}

/**
 * @brief MQTT的UnSubscribe报文
 * 
 * @param topic 订阅的标题
 */
void MQTT_UnSubscribePack(char *topic)
{
    Aliyun_mqtt.Fixed_len = 1;
    Aliyun_mqtt.Variable_len = 2;
    Aliyun_mqtt.Payload_len = 2 + strlen(topic);
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;

    /* 固定报头 */
    Aliyun_mqtt.Pack_buff[0] = UNSUBSCRIBE;    //UnSubscrib报头

    //判断剩余长度是一个字节还是两个字节
    MQTT_Remaining_Len_Process();

    /* 可变报头 */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = Aliyun_mqtt.MessageID/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = Aliyun_mqtt.MessageID%256;
    Aliyun_mqtt.MessageID++;

    /* 有效负载 */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2] = strlen(topic)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3] = strlen(topic)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4],topic,strlen(topic));

    if(DTU_SendData(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len))
        u1_printf("UnSubscribe报文发送成功,等待服务器回应\r\n");
}

/**
 * @brief MQTT的处理Publish报文（等级0）
 * 
 * @param data 数据指针
 * @param datalen 数据长度
 */
void MQTT_DealPublishData(uint8_t *data,uint16_t datalen)
{
    uint8_t i;

    //通过与0x80相与，判断剩余长度的位数
    for(i = 1;i < 5;i++)
    {
        if((data[i] & 0x80) == 0)
            break;
    }

    memset(Aliyun_mqtt.CMD_buff,0,512);
    memcpy(Aliyun_mqtt.CMD_buff,&data[1+i+2],datalen-1-i-2);

}

/**
 * @brief MQTT的发送Publish报文（等级0）
 * 
 * @param topic 订阅的标题
 * @param data 发送的数据
 */
void MQTT_PublishDataQos0(char *topic,char *data)
{
    Aliyun_mqtt.Fixed_len = 1;
    Aliyun_mqtt.Variable_len = 2 + strlen(topic);   //等级0没有报文标识符
    Aliyun_mqtt.Payload_len = strlen(data);
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;

    /* 固定报头 */
    Aliyun_mqtt.Pack_buff[0] = PUBLISHQOS0;    //PublishQs0报头

    //判断剩余长度是一个字节还是两个字节
    MQTT_Remaining_Len_Process();

    /* 可变报头 */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = strlen(topic)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = strlen(topic)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2],topic,strlen(topic));

    /* 有效负载 */
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2 + strlen(topic)],data,strlen(data));

    DTU_SendData(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len);
}

/**
 * @brief MQTT的发送Publish报文（等级1）
 * 
 * @param topic 订阅的标题
 * @param data 发送的数据
 */
void MQTT_PublishDataQos1(char *topic,char *data)
{
    Aliyun_mqtt.Fixed_len = 1;
    Aliyun_mqtt.Variable_len = 2 + strlen(topic) + 2;
    Aliyun_mqtt.Payload_len = strlen(data);
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;

    /* 固定报头 */
    Aliyun_mqtt.Pack_buff[0] = PUBLISHQOS1;    //PublishQos1报头

    //判断剩余长度是一个字节还是两个字节
    MQTT_Remaining_Len_Process();

    /* 可变报头 */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = strlen(topic)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = strlen(topic)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2],topic,strlen(topic));

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2 + strlen(topic)] = Aliyun_mqtt.MessageID/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3 + strlen(topic)] = Aliyun_mqtt.MessageID%256;
    Aliyun_mqtt.MessageID++;

    /* 有效负载 */
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4 + strlen(topic)],data,strlen(data));

    DTU_SendData(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len);
}

