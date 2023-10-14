#include "main.h"
#include "stdio.h"
#include "usart.h"
#include "string.h"
#include "mqtt.h"
#include "dtu-4g.h"
#include "sort.h"

/* MQTT���ƿ� */
MQTT_CB Aliyun_mqtt;

extern struct DeviceInfo Dev;

/**
 * @brief MQTTʣ�೤���ֽ����жϴ���
 * 
 */
static void MQTT_Remaining_Len_Process(void)
{
    //�ж�ʣ�೤����һ���ֽڻ��������ֽ�
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
 * @brief MQTT��Connect����
 * 
 */
void MQTT_ConnectPack(void)
{
    Aliyun_mqtt.MessageID = 1;
    Aliyun_mqtt.Fixed_len = 1;
    Aliyun_mqtt.Variable_len = 10;
    Aliyun_mqtt.Payload_len = 2 + strlen(Dev.clientId) + 2 + strlen(Dev.username) + 2 + strlen(Dev.passwd);   //���е�2����ʾΪ����
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;

    /* �̶���ͷ */
    Aliyun_mqtt.Pack_buff[0] = CONNECT;    //Connect��ͷ

    //�ж�ʣ�೤����һ���ֽڻ��������ֽ�
    MQTT_Remaining_Len_Process();

    /* �ɱ䱨ͷ */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = 0x00;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = 0x04;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2] = 0x4D;    //'M'
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3] = 0x51;    //'Q'
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4] = 0x54;    //'T'
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 5] = 0x54;    //'T'
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 6] = 0x04;    //Э�鼶��04
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 7] = 0xC2;    //���ӱ�־
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 8] = 0x00;    //��������ʱ����ֽڣ�100��
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 9] = 0x64;    //��������ʱ����ֽڣ�100��

    /* ��Ч���� */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 10] = strlen(Dev.clientId)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 11] = strlen(Dev.clientId)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 12],Dev.clientId,strlen(Dev.clientId));

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 12 + strlen(Dev.clientId)] = strlen(Dev.username)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 13 + strlen(Dev.clientId)] = strlen(Dev.username)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 14 + strlen(Dev.clientId)],Dev.username,strlen(Dev.username));

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 14 + strlen(Dev.clientId) + strlen(Dev.username)] = strlen(Dev.passwd)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 15 + strlen(Dev.clientId) + strlen(Dev.username)] = strlen(Dev.passwd)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 16 + strlen(Dev.clientId) + strlen(Dev.username)],Dev.passwd,strlen(Dev.passwd));

    //�����������
    // for(int i = 0;i < (Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len);i++)
    //     u1_printf("%02x ",Aliyun_mqtt.Pack_buff[i]);
    //HAL_UART_Transmit(&DTU_USART,Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len,0xffff);

    /* ʹ��DTU����Connect���ĸ������� */
    if(DTU_SendData(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len))
        u1_printf("Connect���ķ��ͳɹ�,�ȴ���������Ӧ\r\n");

}

/**
 * @brief MQTT��Subscribe����
 * 
 */
void MQTT_SubscribePack(char *topic)
{
    Aliyun_mqtt.Fixed_len = 1;
    Aliyun_mqtt.Variable_len = 2;
    Aliyun_mqtt.Payload_len = 2 + strlen(topic) + 1;   //���е�2����ʾΪ����,1������������ȼ�
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;

    /* �̶���ͷ */
    Aliyun_mqtt.Pack_buff[0] = SUBSCRIBE;    //Subscrib��ͷ

    //�ж�ʣ�೤����һ���ֽڻ��������ֽ�
    MQTT_Remaining_Len_Process();

    /* �ɱ䱨ͷ */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = Aliyun_mqtt.MessageID/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = Aliyun_mqtt.MessageID%256;
    Aliyun_mqtt.MessageID++;

    /* ��Ч���� */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2] = strlen(topic)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3] = strlen(topic)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4],topic,strlen(topic));

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4 + strlen(topic)] = 0;   //���������ȼ�Ϊ0

    if(DTU_SendData(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len))
        u1_printf("Subscribe���ķ��ͳɹ�,�ȴ���������Ӧ\r\n");
}

/**
 * @brief MQTT��UnSubscribe����
 * 
 * @param topic ���ĵı���
 */
void MQTT_UnSubscribePack(char *topic)
{
    Aliyun_mqtt.Fixed_len = 1;
    Aliyun_mqtt.Variable_len = 2;
    Aliyun_mqtt.Payload_len = 2 + strlen(topic);
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;

    /* �̶���ͷ */
    Aliyun_mqtt.Pack_buff[0] = UNSUBSCRIBE;    //UnSubscrib��ͷ

    //�ж�ʣ�೤����һ���ֽڻ��������ֽ�
    MQTT_Remaining_Len_Process();

    /* �ɱ䱨ͷ */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = Aliyun_mqtt.MessageID/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = Aliyun_mqtt.MessageID%256;
    Aliyun_mqtt.MessageID++;

    /* ��Ч���� */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2] = strlen(topic)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3] = strlen(topic)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4],topic,strlen(topic));

    if(DTU_SendData(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len))
        u1_printf("UnSubscribe���ķ��ͳɹ�,�ȴ���������Ӧ\r\n");
}

/**
 * @brief MQTT�Ĵ���Publish���ģ��ȼ�0��
 * 
 * @param data ����ָ��
 * @param datalen ���ݳ���
 */
void MQTT_DealPublishData(uint8_t *data,uint16_t datalen)
{
    uint8_t i;

    //ͨ����0x80���룬�ж�ʣ�೤�ȵ�λ��
    for(i = 1;i < 5;i++)
    {
        if((data[i] & 0x80) == 0)
            break;
    }

    memset(Aliyun_mqtt.CMD_buff,0,512);
    memcpy(Aliyun_mqtt.CMD_buff,&data[1+i+2],datalen-1-i-2);

}

/**
 * @brief MQTT�ķ���Publish���ģ��ȼ�0��
 * 
 * @param topic ���ĵı���
 * @param data ���͵�����
 */
void MQTT_PublishDataQos0(char *topic,char *data)
{
    Aliyun_mqtt.Fixed_len = 1;
    Aliyun_mqtt.Variable_len = 2 + strlen(topic);   //�ȼ�0û�б��ı�ʶ��
    Aliyun_mqtt.Payload_len = strlen(data);
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;

    /* �̶���ͷ */
    Aliyun_mqtt.Pack_buff[0] = PUBLISHQOS0;    //PublishQs0��ͷ

    //�ж�ʣ�೤����һ���ֽڻ��������ֽ�
    MQTT_Remaining_Len_Process();

    /* �ɱ䱨ͷ */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = strlen(topic)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = strlen(topic)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2],topic,strlen(topic));

    /* ��Ч���� */
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2 + strlen(topic)],data,strlen(data));

    DTU_SendData(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len);
}

/**
 * @brief MQTT�ķ���Publish���ģ��ȼ�1��
 * 
 * @param topic ���ĵı���
 * @param data ���͵�����
 */
void MQTT_PublishDataQos1(char *topic,char *data)
{
    Aliyun_mqtt.Fixed_len = 1;
    Aliyun_mqtt.Variable_len = 2 + strlen(topic) + 2;
    Aliyun_mqtt.Payload_len = strlen(data);
    Aliyun_mqtt.Remaining_len = Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len;

    /* �̶���ͷ */
    Aliyun_mqtt.Pack_buff[0] = PUBLISHQOS1;    //PublishQos1��ͷ

    //�ж�ʣ�೤����һ���ֽڻ��������ֽ�
    MQTT_Remaining_Len_Process();

    /* �ɱ䱨ͷ */
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 0] = strlen(topic)/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 1] = strlen(topic)%256;
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2],topic,strlen(topic));

    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 2 + strlen(topic)] = Aliyun_mqtt.MessageID/256;
    Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 3 + strlen(topic)] = Aliyun_mqtt.MessageID%256;
    Aliyun_mqtt.MessageID++;

    /* ��Ч���� */
    memcpy(&Aliyun_mqtt.Pack_buff[Aliyun_mqtt.Fixed_len + 4 + strlen(topic)],data,strlen(data));

    DTU_SendData(Aliyun_mqtt.Pack_buff,Aliyun_mqtt.Fixed_len + Aliyun_mqtt.Variable_len + Aliyun_mqtt.Payload_len);
}

