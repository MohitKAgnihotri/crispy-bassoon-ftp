#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include "server.h"

#define BACKLOG 10
#define PORT_NUM 1313




/* Thread routine to serve connection to client. */
void *pthread_routine(void *arg);

/* Signal handler to handle SIGTERM and SIGINT signals. */
void signal_handler(int signal_number);

void SetupSignalHandler();

int CreateServerSocket(int port);

int server_socket_fd;

static auth_info_t authInfo[MAX_USERS] = {{0},{0}};
static int number_of_users = 0;

void PopulateAuthenticationInformation(const char *authFile) {
    char buf[1024] = {0};
    char username[255] = {0}, password[255] = {0};
    FILE *fp = fopen(authFile, "r");
    if (fp) {
        while (fgets(buf, sizeof(buf), fp) != NULL)
        {
            sscanf(buf, "%s %s", authInfo[number_of_users].username, authInfo[number_of_users].password);
            printf("Username = %s \t password = %s", username, password);
            number_of_users++;
        }
    }
}

bool isValidPassword(const char* username, const char* pass)
{
    for (int i = 0; i < number_of_users; i++)
    {
        if(strcmp(username, authInfo[i].username) == 0 && strcmp(pass, authInfo[i].password))
        {
            return true;
        }
    }
    return false;
}

bool isvalidUserName(const char* username)
{
    for (int i = 0; i < number_of_users; i++)
    {
        if(strcmp(username, authInfo[i].username) == 0)
        {
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[]) {
    int port, new_socket_fd;
    pthread_attr_t pthread_client_attr;
    pthread_t pthread;
    socklen_t client_address_len;
    struct sockaddr_in client_address;
    char *authenticationFile = NULL;

    /* Get port from command line arguments or stdin.
     */

    if (argc < 3) {
        printf("Incorrect Usage\n");
        printf("%s <PORT_NUMBER> <Authentication_File.txt>", argv[0]);
    }

    port = atoi(argv[1]);

    authenticationFile = argv[2];

    PopulateAuthenticationInformation(authenticationFile);

    /*Create the server socket */
    server_socket_fd = CreateServerSocket(port);

    /*Setup the signal handler*/
    SetupSignalHandler();

    /* Initialise pthread attribute to create detached threads. */
    if (pthread_attr_init(&pthread_client_attr) != 0) {
        perror("pthread_attr_init");
        exit(1);
    }
    if (pthread_attr_setdetachstate(&pthread_client_attr, PTHREAD_CREATE_DETACHED) != 0) {
        perror("pthread_attr_setdetachstate");
        exit(1);
    }

    while (1) {

        /* Accept connection to client. */
        client_address_len = sizeof(client_address);
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

}

void signal_handler(int signal_number) {
    close(server_socket_fd);
    exit(0);
}