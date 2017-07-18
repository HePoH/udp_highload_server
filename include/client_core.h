#ifndef CLIENT_CORE_H
#define CLIENT_CORE_H

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <arpa/inet.h>

#define SERVER_IP INADDR_LOOPBACK
#define SERVER_PORT 8888

#define MAX_REQS_MSG_SIZE 64
#define MAX_REPL_MSG_SIZE 128

#endif
