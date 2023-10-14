#ifndef __SORT_H_
#define __SORT_H_

#include "stdint.h"
#include "stdio.h"
#include "string.h"

/* 设备信息结构体 */
struct DeviceInfo{
    char clientId[100];
    char username[30];
    char passwd[100];
    char ProductKey[30];
    char DeviceName[30];
};

void Sort_DeviceInfo(void);

#endif
