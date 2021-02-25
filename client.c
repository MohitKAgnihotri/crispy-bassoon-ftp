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
#include "client.h"






#define SERVER_NAME_LEN_MAX 255

int get_user_input(char *buffer){
    //clear buffer
    memset(buffer, 0, (int)sizeof(buffer));

    //print prompt
    printf("ftp> ");

    //get user input
    if(fgets(buffer, 1024, stdin) == NULL)
    {
        return CLIENT_FAILURE;
    }

    buffer[strcspn(buffer, "\r\n")] = 0;
    return CLIENT_SUCCESS;
}

char VALID_FTP_COMMANDS[][32] = { {"USER"},
                                  {"PASS"},
                                  {"PUT"},
                                  {"GET"},
                                  {"!LS"},
                                  {"CD"},
                                  {"!CD"},
                                  {"PWD"},
                                  {"!PWD"},
                                  {"QUIT"}
};


void handle_USER_Command(char *buffer, int socketfd)
{
    printf("\n USER");
}

void handle_PASS_Command(char *buffer, int socketfd)
{
    printf("\n PASS");
}

void handle_PUT_Command(char *buffer, int socketfd)
{
    printf("\n PUT");
}

void handle_GET_Command(char *buffer, int socketfd)
{
    printf("\n GET");
}

void handle_LS_REMOTE_Command(char *buffer, int socketfd)
{
    printf("\n LS_REMOTE");
}


void handle_PWD_REMOTE_Command(char *buffer, int socketfd)
{
    printf("\n PWD_REMOTE");
}

void handle_PWD_Command(char *buffer, int socketfd)
{
    printf("\n PWD");
}

void handle_CD_REMOTE_Command(char *buffer, int socketfd)
{
    printf("\n CD_REMOTE");
}

void handle_CD_Command(char *buffer, int socketfd)
{
    printf("\n CD");
}

void handle_QUIT_Command(char *buffer, int socketfd)
{
    printf("\n QUIT");
}

ftp_command supportedFtpCommands[10] = {handle_USER_Command,
                                        handle_PASS_Command,
                                        handle_PUT_Command,
                                        handle_GET_Command,
                                        handle_LS_REMOTE_Command,
                                        handle_PWD_REMOTE_Command,
                                        handle_PWD_Command,
                                        handle_CD_REMOTE_Command,
                                        handle_CD_Command,
                                        handle_QUIT_Command

};


int isValidCommand(const char* command)
{
    for (int i = 0; i < 10; i++)
    {
        if(strcmp(VALID_FTP_COMMANDS[i], command) == 0)
        {
            return i;
        }
    }
    return CLIENT_INVALID_COMMAND;
}



int main(int argc, char *argv[])
{
    char server_name[SERVER_NAME_LEN_MAX + 1] = { 0 };
    int server_port, socket_fd;
    struct hostent *server_host;
    struct sockaddr_in server_address;
    char message_received[300];

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

    bool isSessionEnd = false;

    int commandIdx = CLIENT_INVALID_COMMAND;

    while(!isSessionEnd) {
        char message_buffer[1024];
        get_user_input(message_buffer);

        while ((commandIdx = isValidCommand(message_buffer)) == CLIENT_INVALID_COMMAND)
        {
            printf("ftp> incorrect command. try again");
            get_user_input(message_buffer);
        }

        supportedFtpCommands[commandIdx](message_buffer, socket_fd);


    }

    close(socket_fd);
    return 0;
}