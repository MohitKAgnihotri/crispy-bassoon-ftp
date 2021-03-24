//
// Created by 310165137 on 25/02/2021.
//


#include "utility.h"

#ifndef CRISPY_BASSOON_FTP_SERVER_H
#define CRISPY_BASSOON_FTP_SERVER_H

typedef struct auth_info
{
    char username[255];
    char password[255];
}auth_info_t;


typedef struct activeUserInfo
{
    bool isInuse;
    int socketfd;
    ftpConnectionCB_t connCb;
}activeUserInfo_t;

#define MAX_USERS 100

#endif //CRISPY_BASSOON_FTP_SERVER_H
