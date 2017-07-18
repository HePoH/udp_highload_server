#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#define _SVID_SOURCE
#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include <netdb.h>

#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include <arpa/inet.h>

#define SERVER_IP INADDR_ANY
#define SERVER_PORT 8888

#define REPL_THREAD 10

#define REQS_REPL_KEY_FILE "/tmp/highload_server_queue/mqueue_reqs_repl_key"
#define PROJECT_ID 'N'
#define QUEUE_PERMISSIONS 0666

#define MAX_REQS_MSG_SIZE 64
#define MAX_REPL_MSG_SIZE 128

#define MAX_LOG_MSG_SIZE 128

#define REQS_MSG_TYPE 1L
#define REPL_MSG_TYPE 2L

#define REQS_REPL_MSG_SIZE sizeof(REQS_REPL_MSG)

enum LOG_MSG_TYPE {
        INFO,
        WARNING,
        ERROR
};

typedef struct reqs_repl_msg {
	long msg_type;

	struct sockaddr_in cln_addr;
	socklen_t socket_len;

        char request[MAX_REQS_MSG_SIZE];
        char reply[MAX_REPL_MSG_SIZE];
} REQS_REPL_MSG;

char* get_cur_dt(char* format);
void sys_log(char* msg, int type, int fd);

void make_key();
void destructor(void* ptr);

void init_socket(int* sd_srv, struct sockaddr_in* srv_addr);
void init_queue(int* reqs_repl_qid);

void srv_reqs_hndl(int sd_srv, int reqs_repl_qid);
void* cln_repl_hndl(void* args);

#endif
