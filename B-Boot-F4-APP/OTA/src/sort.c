#include "sort.h"
#include "usart.h"
#include "ymodem.h"

/* 设备信息结构体 */
struct DeviceInfo Dev;

/**
 * @brief 根据bootloader中的设备符选择对应的设备信息，将所有设备的信息填入switch中
 */
void Sort_DeviceInfo(void)
{
    uint32_t No;

    //读取bootloader最后32位，存储到No中
    Flash_Read((Application_2_Addr + Application_Size),&No,1);         

    //根据读取到的序号值，去给设备信息结构体赋值，1对应D001
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

