
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

supportedFtpCommands_t supportedFtpCommands[MAX_SUPPORTED_COMMANDS] = {{"USER",handle_USER_Command},
                                                   {"PASS",handle_PASS_Command},
                                                   {"PUT",handle_PUT_Command},
                                                   {"GET",handle_GET_Command},
                                                   {"!LS",handle_LS_Command},
                                                   {"LS",handle_LS_REMOTE_Command},
                                                   {"!PWD",handle_PWD_Command},
                                                   {"PWD",handle_PWD_REMOTE_Command},
                                                   {"!CD",handle_CD_Command},
                                                   {"CD",handle_CD_REMOTE_Command},
                                                   {"QUIT",handle_QUIT_Command}

};

int get_user_input(char **input)
{
    *input = (char*) malloc(MAX_ARG_SIZE * sizeof(char));

    //clear buffer
    memset(*input, 0, MAX_ARG_SIZE);

    //print prompt
    printf("ftp>");

    //get user input
    if(fgets(*input, MAX_ARG_SIZE, stdin) == NULL)
    {
        return FAILURE;
    }

    //trim trailing
    *input[strcspn(*input, "\r\n")] = 0;
    return SUCCESS;
}

int get_user_command(char **command, char **argument)
{
    char user_buffer[1024] = {0};
    char *command_buf = (char*) malloc(MAX_CMD_SIZE * sizeof(char));
    char *arg_buf = (char*) malloc(MAX_ARG_SIZE * sizeof(char));
    //clear buffer
    memset(command_buf, 0, MAX_CMD_SIZE);
    memset(arg_buf, 0, MAX_ARG_SIZE);

    //print prompt
    printf("ftp>");

    //get user input
    if(fgets(user_buffer, MAX_USER_INPUT_SIZE, stdin) == NULL)
    {
        return FAILURE;
    }

    //trim trailing
    user_buffer[strcspn(user_buffer, "\r\n")] = 0;

    sscanf(user_buffer,"%s %s",command_buf,arg_buf);

    *command = command_buf;
    *argument = arg_buf;
    return SUCCESS;
}

int isValidCommand(const char* command)
{
    for (int i = 0; i < 10; i++)
    {
        if(strcmp(supportedFtpCommands[i].commandName, command) == 0)
        {
            return i;
        }
    }
    return INVALID_COMMAND;
}
