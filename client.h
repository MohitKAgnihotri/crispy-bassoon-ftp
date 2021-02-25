//
// Created by 310165137 on 25/02/2021.
//

#ifndef CRISPY_BASSOON_FTP_CLIENT_H
#define CRISPY_BASSOON_FTP_CLIENT_H


#define CLIENT_SUCCESS 1u
#define CLIENT_FAILURE 0u
#define CLIENT_INVALID_COMMAND -1


typedef void (*ftp_command) (char * buffer , int socket);

#endif //CRISPY_BASSOON_FTP_CLIENT_H
