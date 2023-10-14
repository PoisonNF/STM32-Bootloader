#ifndef __MQTT_H_
#define __MQTT_H_

#include "stdint.h"

/* �豸��Ϣ����CONNECTʹ�� */
#if 0
/* MQTTtest��Ϣ */
#define CLIENTID        "k08lcwgm0Ts.MQTTtest|securemode=2,signmethod=hmacsha256,timestamp=1695198813604|"
#define USERNAME        "MQTTtest&k08lcwgm0Ts"
#define PASSWORD        "1f9be4385dde450bd8a4059af5ffccd25e2a6e9a672dd666d7fb2513c593419c"
#define PRODUCTKEY      "k08lcwgm0Ts"
#define DEVICENNAME     "MQTTtest"
#endif

/* D001��Ϣ */
// #define CLIENTID        "k08lcwgm0Ts.D001|securemode=2,signmethod=hmacsha256,timestamp=1697173381876|"
// #define USERNAME        "D001&k08lcwgm0Ts"
// #define PASSWORD        "9df018077e0841352c66b83facc2c9b866f85db233e25d18f9bfc9b9e2d61470"
// #define PRODUCTKEY      "k08lcwgm0Ts"
// #define DEVICENNAME     "D001"

/* ���Ʊ�ͷ */
#define CONNECT         0x10
#define CONNACK         0x20
#define PUBLISHQOS0     0x30
#define PUBLISHQOS1     0x32
#define PUBACK          0x40
#define PUBREC          0x50
#define PUBREL          0x60
#define PUBCOMP         0x70
#define SUBSCRIBE       0x82
#define SUBACK          0x90
#define UNSUBSCRIBE     0xA2
#define UNSUBACK        0xB0
#define PINGREQ         0xC0
#define PINGRESP        0XD0
#define DISCONNECT      0xE0

/* MQTT���ƽṹ�� */
typedef struct{
    //��������
    uint8_t     Pack_buff[512];     //�豸����������Buffer
    uint8_t     CMD_buff[512];      //���������豸��Buffer
    uint16_t    MessageID;          //���ı�ʶ��
    uint16_t    Fixed_len;          //�̶���ͷ����
    uint16_t    Variable_len;       //�ɱ䱨ͷ����
    uint16_t    Payload_len;        //���ر��ĳ���
    uint16_t    Remaining_len;      //ʣ�೤��

    //OTA��Ϣ
    int size;                       //�̼�����С
    int streamId;                   //�̼���ID
    uint8_t OTA_VerTemp[10];        //�汾����ʱ���

    int counter;                    //�ƻ���������
    int num;                        //��ǰ���ش���
    int downlen;                    //�������ش�С
}MQTT_CB;

extern MQTT_CB Aliyun_mqtt;


void MQTT_ConnectPack(void);

void MQTT_SubscribePack(char *topic);

void MQTT_UnSubscribePack(char *topic);

void MQTT_DealPublishData(uint8_t *data,uint16_t datalen);

void MQTT_PublishDataQos0(char *topic,char *data);

void MQTT_PublishDataQos1(char *topic,char *data);

#endif // !__MQTT_H_

