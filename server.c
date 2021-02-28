#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "utility.h"
#include "server.h"


#define BACKLOG 10
#define PORT_NUM 1313


extern supportedFtpCommands_t supportedFtpCommands[10];

bool isValidUserName(const char *username);

bool isValidPassword(const char *username, const char *pass);


/* Thread routine to serve connection to client. */
void *pthread_routine(void *arg);

/* Signal handler to handle SIGTERM and SIGINT signals. */
void signal_handler(int signal_number);

void SetupSignalHandler();

int CreateServerSocket(int port);

int server_socket_fd;

static auth_info_t authInfo[MAX_USERS] = {{0},
                                          {0}};
static int number_of_users = 0;

int SendResponse(size_t len, char *message, int socket) {
    size_t writeLen = write(socket, message, len);
    if (writeLen <= 0) {
        perror("SendResponse");
    }
}


void sendErrorMessage(int fd, char *string) {
    char message[1024] = {0u};
    snprintf(message, 1024, "%s %s", "Invalid Command", string);
    size_t write_len = write(fd, message, 1024);
    if (write_len <= 0) {
        printf("ftp>Connection closed\n");
        printf("ftp>bye\n");
    }
}


void handle_USER_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd) {
    if (isValidUserName(buffer)) {
        SendResponse(sizeof("Username OK, password required"), "Username OK, password required", socketfd);
        connCB->connState |= USERNAME_VERIFIED;
        memcpy(connCB->argument, buffer, MAX_ARG_SIZE);
    } else {
        SendResponse(sizeof("Username does not exist"), "Username does not exist", socketfd);
    }
}

void handle_PASS_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd) {
    if (connCB->connState & USERNAME_VERIFIED) {
        if (isValidPassword(connCB->argument, buffer)) {
            SendResponse(sizeof("Authentication complete"), "Authentication complete", socketfd);
            connCB->connState |= USERNAME_AUTHENTICATED;
            memcpy(connCB->argument, buffer, MAX_ARG_SIZE);
        } else {
            SendResponse(sizeof("Wrong password"), "Wrong Password", socketfd);
        }
    }
}

void handle_PUT_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd) {
    printf("\n PUT");
}

void handle_GET_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd) {
    printf("\n GET");
}

void handle_LS_REMOTE_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd) {
    printf("\n LS_REMOTE");
}

void handle_PWD_REMOTE_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd) {
    printf("\n PWD_REMOTE");
}

void handle_PWD_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd) {
    printf("\n PWD");
}

void handle_CD_REMOTE_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd) {
    printf("\n CD_REMOTE");
}

void handle_CD_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd) {
    printf("\n CD");
}

void handle_QUIT_Command(ftpConnectionCB_t *connCB, char *buffer, int socketfd) {
    printf("ftp>bye\n");
    exit(0);
}

void PopulateAuthenticationInformation(const char *authFile) {
    char buf[1024] = {0};
    FILE *fp = fopen(authFile, "r");
    if (fp) {
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            sscanf(buf, "%s %s", authInfo[number_of_users].username, authInfo[number_of_users].password);
            number_of_users++;
        }
    }
}

bool isValidPassword(const char *username, const char *pass) {
    for (int i = 0; i < number_of_users; i++) {
        if (strcmp(username, authInfo[i].username) == 0 && strcmp(pass, authInfo[i].password) == 0) {
            return true;
        }
    }
    return false;
}

bool isValidUserName(const char *username) {
    for (int i = 0; i < number_of_users; i++) {
        if (strcmp(username, authInfo[i].username) == 0) {
            return true;
        }
    }
    return false;
}

int read_from_client(activeUserInfo_t *controlBlock, int filedes)
{
    char buffer[MAX_SOCKET_MSG_LEN_SIZE];
    int nbytes;

    nbytes = read(filedes, buffer, MAX_SOCKET_MSG_LEN_SIZE);
    if (nbytes < 0)
    {
        /* Read error. */
        perror("read");
        exit(EXIT_FAILURE);
    }
    else if  (nbytes == 0)
    {
        /* End-of-file. */
        return -1;
    }
    else
    {
        int commandIdx = INVALID_COMMAND;
        char command[MAX_CMD_SIZE] = {0}, argument[MAX_ARG_SIZE] = {0};
        sscanf(buffer, "%s %s", command, argument);
        if ((commandIdx = isValidCommand(command)) == INVALID_COMMAND) {
            sendErrorMessage(filedes, command);
        }
        supportedFtpCommands[commandIdx].pfnCommandHandler(&controlBlock->connCb, argument, filedes);
        return 0;
    }
}

unsigned int AllocatedActiveUuserControlBlock(activeUserInfo_t *activeUserControlBlocks) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (activeUserControlBlocks[i].isFree) {
            return i;
        }
    }
    return MAX_USERS;
}

void FreeActiveUuserControlBlock(activeUserInfo_t *activeUserControlBlocks, unsigned int userIndex) {
    if (userIndex < MAX_USERS && activeUserControlBlocks) {
        activeUserControlBlocks[userIndex].isFree = true;
        activeUserControlBlocks[userIndex].socketfd = -1;
        memset(&activeUserControlBlocks[userIndex].connCb, 0x00, sizeof(ftpConnectionCB_t));
    }
}


unsigned int getActiveUserCb(int socketFd, activeUserInfo_t *activeUserControlBlocks) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (activeUserControlBlocks[i].socketfd == socketFd) {
            return i;
        }
    }
    return MAX_USERS;
}


int main(int argc, char *argv[]) {
    int port, new_socket_fd;
    pthread_attr_t pthread_client_attr;
    pthread_t pthread;

    struct sockaddr_in client_address;
    char *authenticationFile = NULL;
    socklen_t size;


    // Select() variables
    fd_set active_fd_set, read_fd_set;
    int connection_fd_range;
    FD_ZERO(&active_fd_set);
    FD_ZERO(&read_fd_set);

    /* Get port from command line arguments or stdin.
     */

    if (argc < 3) {
        printf("Incorrect Usage\n");
        printf("%s <PORT_NUMBER> <Authentication_File.txt>\n", argv[0]);
        exit(0);
    }

    port = atoi(argv[1]);

    authenticationFile = argv[2];

    PopulateAuthenticationInformation(authenticationFile);

    /*Create the server socket */
    server_socket_fd = CreateServerSocket(port);
    FD_SET(server_socket_fd, &active_fd_set);
    printf("ftp> Server Started at localhost:8080 \n");

    /*Setup the signal handler*/
    SetupSignalHandler();

    /*Setup active user database*/
    activeUserInfo_t activeUser[MAX_USERS] = {0};
#if 0
    /* Initialise pthread attribute to create detached threads. */
    if (pthread_attr_init(&pthread_client_attr) != 0) {
        perror("pthread_attr_init");
        exit(1);
    }
    if (pthread_attr_setdetachstate(&pthread_client_attr, PTHREAD_CREATE_DETACHED) != 0) {
        perror("pthread_attr_setdetachstate");
        exit(1);
    }

    while (1)
    {

        /* Accept connection to client. */
        socklen_t client_address_len = sizeof(client_address);
        new_socket_fd = accept(server_socket_fd, (struct sockaddr *) &client_address, &client_address_len);
        if (new_socket_fd == -1) {
            perror("accept");
            continue;
        }

        printf("Client connected\n");
        unsigned int *thread_arg = (unsigned int *) malloc(sizeof(unsigned int));
        *thread_arg = new_socket_fd;
        /* Create thread to serve connection to client. */
        if (pthread_create(&pthread, &pthread_client_attr, pthread_routine, (void *) thread_arg) != 0) {
            perror("pthread_create");
            continue;
        }
    }
#endif

    while (1) {
        unsigned int userIndex = 0;
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(EXIT_FAILURE);
        }

        /* Service all the sockets with input pending. */
        for (int i = 0; i < FD_SETSIZE; ++i)
            if (FD_ISSET (i, &read_fd_set)) {
                if (i == server_socket_fd) {
                    /* Connection request on original socket. */
                    int client_socket;
                    size = sizeof(client_address);
                    client_socket = accept(server_socket_fd,(struct sockaddr *) &client_address, &size);
                    if (client_socket < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }

                    fprintf(stderr,
                            "Server: connect from host %s, port %hd.\n",
                            inet_ntoa(client_address.sin_addr),
                            ntohs(client_address.sin_port));

                    FD_SET (client_socket, &active_fd_set);
                    if ((userIndex = AllocatedActiveUuserControlBlock(activeUser)) != MAX_USERS)
                    {
                        //control block successfully allocated
                        activeUser[userIndex].isFree = false;
                        activeUser[userIndex].socketfd = client_socket;
                        activeUser[userIndex].connCb.connState |= CONNECTED;
                    }
                }
                else
                {
                    /* Data arriving on an already-connected socket. */
                    if (read_from_client(&activeUser[getActiveUserCb(i, activeUser)], i) < 0)
                    {
                        close(i);
                        FD_CLR (i, &active_fd_set);
                        FreeActiveUuserControlBlock(activeUser, getActiveUserCb(i, activeUser));
                    }
                }
            }
    }

    return 0;
}


int CreateServerSocket(int port) {
    struct sockaddr_in address;
    int socket_fd;

    /* Initialise IPv4 address. */
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* Bind address to socket. */
    if (bind(socket_fd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        perror("bind");
        exit(1);
    }

    /* Listen on socket. */
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // Configure server socket
    int enable = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
    return socket_fd;
}

void SetupSignalHandler() {/* Assign signal handlers to signals. */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
}


void *pthread_routine(void *arg) {
    int scoketfd = *(int *) arg;
    free(arg);
    char message_buffer[1024] = {0};
    ftpConnectionCB_t connectionCB = {0};

    connectionCB.connState |= CONNECTED;
    while (1) {
        int commandIdx = INVALID_COMMAND;
        memset(message_buffer, 0x00, 1024);
        size_t readlen = read(scoketfd, message_buffer, 1024);
        if (readlen > 0) {
            char command[MAX_CMD_SIZE] = {0}, argument[MAX_ARG_SIZE] = {0};
            sscanf(message_buffer, "%s %s", command, argument);
            if ((commandIdx = isValidCommand(command)) == INVALID_COMMAND) {
                sendErrorMessage(scoketfd, command);
            }
            supportedFtpCommands[commandIdx].pfnCommandHandler(&connectionCB, argument, scoketfd);
        } else {
            if (readlen == 0) {
                printf("ftp>Client Disconnected\n");
                pthread_exit(0);
            } else {
                perror("read failed\n");
                pthread_exit(0);
            }
        }
    }

}

void signal_handler(int signal_number) {
    close(server_socket_fd);
    exit(0);
}