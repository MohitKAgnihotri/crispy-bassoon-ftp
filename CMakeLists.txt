cmake_minimum_required(VERSION 3.19)
project(crispy_bassoon_ftp C)

set(CMAKE_C_STANDARD 11)
link_libraries(pthread)
set(CMAKE_C_FLAGS "-g3 -Wall -Werror -Wpedantic")

include_directories(.)

add_executable(server_ftp
        server.c
        server.h utility.c utility.h)

add_executable(client_ftp
        client.c
        client.h utility.c utility.h)
