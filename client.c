//
// Created by 310165137 on 25/02/2021.
//


#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include "utility.h"
#include "client.h"

#define SERVER_NAME_LEN_MAX 255

extern supportedFtpCommands_t supportedFtpCommands[10];

ftpConnectionCB_t connectCb;

void handle_USER_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd)
{
    char server_message[1024] = {0};
    snprintf(server_message, MAX_SOCKET_MSG_LEN_SIZE,"%s %s","USER", buffer);
    size_t write_len = write (socketfd,server_message,strlen(server_message)+1);
    if (write_len == 0)
    {
        printf("ftp>disconnected\n");
        printf("ftp>bye\n");
        exit(0);
    }

    size_t read_len = read(socketfd,server_message, 1024);
    if (read_len == 0)
    {
        printf("ftp>disconnected\n");
        printf("ftp>bye\n");
        exit(0);
    }

    //Received response from the server
    printf("ftp>%s\n",server_message);
    if (strncmp(server_message,"Username OK, password required",sizeof("Username OK, password required")) == 0)
    {
        connCB->connState |= USERNAME_VERIFIED;
    }
}

void handle_PASS_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd)
{
    if (connCB->connState & USERNAME_VERIFIED) {
        char server_message[1024] = {0};
        snprintf(server_message, MAX_SOCKET_MSG_LEN_SIZE, "%s %s", "PASS", buffer);
        size_t write_len = write(socketfd, server_message, strlen(server_message) + 1);
        if (write_len == 0) {
            printf("ftp>disconnected\n");
            printf("ftp>bye\n");
            exit(0);
        }

        size_t read_len = read(socketfd, server_message, 1024);
        if (read_len == 0) {
            printf("ftp>disconnected\n");
            printf("ftp>bye\n");
            exit(0);
        }

        //Received response from the server
        printf("ftp>%s\n", server_message);
        if (strncmp(server_message, "Username OK, password required", sizeof("Username OK, password required")) == 0) {
            connCB->connState |= USERNAME_AUTHENTICATED;
        }
    } else {
        printf("ftp> Username is not set\n");
    }
}

void handle_PUT_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd)
{
    printf("\n PUT");
}

void handle_GET_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd)
{
    printf("\n GET");
}

void handle_LS_REMOTE_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd)
{
    printf("\n LS_REMOTE");
}


void handle_PWD_REMOTE_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd)
{
    printf("\n PWD_REMOTE");
}

void handle_PWD_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd)
{
    printf("\n PWD");
}

void handle_CD_REMOTE_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd)
{
    printf("\n CD_REMOTE");
}

void handle_CD_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd)
{
    printf("\n CD");
}

void handle_QUIT_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd)
{
    printf("ftp>bye\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    char server_name[SERVER_NAME_LEN_MAX + 1] = { 0 };
    int server_port, socket_fd;
    struct hostent *server_host;
    struct sockaddr_in server_address;

    /* Get server name from command line arguments or stdin. */
    if (argc > 1)
    {
        strncpy(server_name, argv[1], SERVER_NAME_LEN_MAX);
    }
    else
    {
        printf("Enter Server Name: ");
        scanf("%s", server_name);
    }

    /* Get server port from command line arguments or stdin. */
    server_port = argc > 2 ? atoi(argv[2]) : 0;
    if (!server_port)
    {
        printf("Enter Port: ");
        scanf("%d", &server_port);
    }

    /* Get server host from server name. */
    server_host = gethostbyname(server_name);

    /* Initialise IPv4 server address with server host. */
    memset(&server_address, 0, sizeof server_address);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* Connect to socket with server address. */
    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
        perror("connect");
        exit(1);
    }

    connectCb.connState = CONNECTED;

    bool isSessionEnd = false;

    int commandIdx = INVALID_COMMAND;

    while(!isSessionEnd) {
        char *command, *argument;

        get_user_command(&command, &argument);
        while ((commandIdx = isValidCommand(command)) == INVALID_COMMAND)
        {
            printf("ftp> incorrect command. try again\n");
            free(command); command = NULL;
            free(argument); argument = NULL;
            get_user_command(&command, &argument);
        }
        supportedFtpCommands[commandIdx].pfnCommandHandler(&connectCb, argument, socket_fd);
        free(command); command = NULL;
        free(argument); argument = NULL;
    }

    close(socket_fd);
    return 0;
}