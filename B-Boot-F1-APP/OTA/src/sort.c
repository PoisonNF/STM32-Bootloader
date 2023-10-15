#include "sort.h"
#include "usart.h"
#include "ymodem.h"

/* �豸��Ϣ�ṹ�� */
struct DeviceInfo Dev;

/**
 * @brief ����bootloader�е��豸��ѡ���Ӧ���豸��Ϣ���������豸����Ϣ����switch��
 */
void Sort_DeviceInfo(void)
{
    uint32_t No;

    //��ȡbootloader���32λ���洢��No��
    Flash_Read((Application_2_Addr + Application_Size),&No,1);         

    //���ݶ�ȡ�������ֵ��ȥ���豸��Ϣ�ṹ�帳ֵ��1��ӦD001
    switch(No)
    {
    case 1:
        strcpy(Dev.clientId,"k08lcwgm0Ts.D001|securemode=2,signmethod=hmacsha256,timestamp=1697173381876|");
        strcpy(Dev.username,"D001&k08lcwgm0Ts");
        strcpy(Dev.passwd,"9df018077e0841352c66b83facc2c9b866f85db233e25d18f9bfc9b9e2d61470");
        strcpy(Dev.ProductKey,"k08lcwgm0Ts");
        strcpy(Dev.DeviceName,"D001");
        break;

    case 2:
        strcpy(Dev.clientId,"k08lcwgm0Ts.D002|securemode=2,signmethod=hmacsha256,timestamp=1697182259542|");
        strcpy(Dev.username,"D002&k08lcwgm0Ts");
        strcpy(Dev.passwd,"e8d1107fbacdce80456a790842f2355f77146cb345edafef5638d67a935a24d3");
        strcpy(Dev.ProductKey,"k08lcwgm0Ts");
        strcpy(Dev.DeviceName,"D002");
        break;
    
    default:
        u1_printf("Can't find device!\r\n");
        break;
    }
}

