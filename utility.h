//
// Created by 310165137 on 26/02/2021.
//

#ifndef CRISPY_BASSOON_FTP_UTILITY_H
#define CRISPY_BASSOON_FTP_UTILITY_H

#define MAX_CMD_SIZE 32u
#define MAX_ARG_SIZE 255u
#define MAX_USER_INPUT_SIZE 255u
#define MAX_SOCKET_MSG_LEN_SIZE 1024u

#define SUCCESS 1u
#define FAILURE 0u
#define INVALID_COMMAND (-1)

int isValidCommand(const char* command);
int get_user_command(char **command, char **argument);
int get_user_input(char **input);



typedef enum ftpState
{
    CONNECTED = 1 << 0,
    USERNAME_VERIFIED = 1 << 1,
    USERNAME_AUTHENTICATED= 1 << 2,
    TRANSFER_FILE= 1 << 3,
    RECEIVE_FILE= 1 << 4,
}ftpState_t;

typedef struct ftpConnectionCB
{
    ftpState_t connState;
    char argument[MAX_ARG_SIZE];
}ftpConnectionCB_t;

typedef void (*ftp_command) (ftpConnectionCB_t *connCB, char *buffer, int socketfd);

#endif //CRISPY_BASSOON_FTP_UTILITY_H
