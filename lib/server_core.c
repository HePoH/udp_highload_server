#include "../include/server_core.h"

pthread_key_t key;
pthread_once_t key_once = PTHREAD_ONCE_INIT;

void init_socket(int* sd_srv, struct sockaddr_in* srv_addr) {
	int rtn = 0;
	socklen_t socket_len = 0;

	socket_len = sizeof(struct sockaddr_in);
	memset(srv_addr, 0, socket_len);

	*sd_srv = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if (*sd_srv == -1) {
		perror("Server: socket(server)");

		close(*sd_srv);
		exit(EXIT_FAILURE);
	}

	srv_addr->sin_family = AF_INET;
	srv_addr->sin_addr.s_addr = htonl(SERVER_IP);
	srv_addr->sin_port = htons(SERVER_PORT);

	rtn = bind(*sd_srv, (struct sockaddr*)srv_addr, sizeof(struct sockaddr_in));
	if (rtn == -1) {
		perror("Server: bind(server)");

		close(*sd_srv);
		exit(EXIT_FAILURE);
	}
}

void init_queue(int* reqs_repl_qid) {
	key_t reqs_repl_key;

	reqs_repl_key = ftok(REQS_REPL_KEY_FILE, PROJECT_ID);
	if (reqs_repl_key == -1) {
		perror("Server: ftok(request&replies)");
		exit(EXIT_FAILURE);
	}

        *reqs_repl_qid = msgget(reqs_repl_key, IPC_CREAT | QUEUE_PERMISSIONS);
	if (*reqs_repl_qid == -1) {
		perror("Server: msgget(request&replies)");
		exit(EXIT_FAILURE);
	}
}

void srv_reqs_hndl(int sd_srv, int reqs_repl_qid) {
	struct sockaddr_in cln_addr;
	struct hostent* cln_hst = NULL;
	socklen_t socket_len = 0;
	ssize_t bts = 0;
	char msg[MAX_REQS_MSG_SIZE];
	REQS_REPL_MSG *reqs_msg = NULL, repl_msg;

	socket_len = sizeof(struct sockaddr_in);
	memset(&cln_addr, 0, socket_len);

	while(1) {
		memset(msg, 0, MAX_REQS_MSG_SIZE * sizeof(char));
		usleep(10);

		bts = recvfrom(sd_srv, msg, MAX_REQS_MSG_SIZE, 0, (struct sockaddr*)&cln_addr, &socket_len);
		if (bts == -1) {
			if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
				/*perror("Server: (not error) recvfrom(client)");*/
			}
			else {
				perror("Server: recvfrom(client)");

				close(sd_srv);
				exit(EXIT_FAILURE);
			}
		}
		else {
			cln_hst = gethostbyaddr((char *)&cln_addr.sin_addr.s_addr, 4, AF_INET);
			printf("Server: incoming connection from %s:%d (hs: %s)\n", inet_ntoa(cln_addr.sin_addr), ntohs(cln_addr.sin_port),
			((cln_hst != NULL) ? cln_hst->h_name : ""));

			printf("Server: recived request message from client: '%s' (%d bytes)\n", msg, (int) bts);

			if (!strcmp(msg, "exit")) {
				printf("Server: main thread bye\n");

				close(sd_srv);
				exit(EXIT_SUCCESS);
			}

			reqs_msg = calloc(REQS_REPL_MSG_SIZE, 1);
			if (reqs_msg == NULL) {
				printf("Server: malloc(reqs_msg)");

				close(sd_srv);
				exit(EXIT_SUCCESS);
			}

			reqs_msg->msg_type = REQS_MSG_TYPE;
			memcpy(&reqs_msg->cln_addr, &cln_addr, socket_len);
			reqs_msg->socket_len = socket_len;
			strncpy(reqs_msg->request, msg, bts);

			bts = msgsnd(reqs_repl_qid, (void*)reqs_msg, REQS_REPL_MSG_SIZE, 0);
			if (bts == -1) {
				perror("Server: msgsnd(request)");

				close(sd_srv);
				exit(EXIT_FAILURE);
			}
		}

		memset(&repl_msg, 0, REQS_REPL_MSG_SIZE);

		bts = msgrcv(reqs_repl_qid, (void*)&repl_msg, REQS_REPL_MSG_SIZE, REPL_MSG_TYPE, IPC_NOWAIT);
                if (bts == -1) {
			if (errno == ENOMSG) {
				/*perror("Server: (not error) msgrcv(reply)");*/
			}
			else {
				perror("Server: msgrcv(reply)");

				close(sd_srv);
                        	exit(EXIT_FAILURE);
			}
                }
		else {
			socket_len = sizeof(struct sockaddr_in);
			bts = sendto(sd_srv, repl_msg.reply, strlen(repl_msg.reply), 0, (struct sockaddr*)&repl_msg.cln_addr, repl_msg.socket_len);
			if (bts == -1) {
				perror("Server: sendto(client)");

				close(sd_srv);
				exit(EXIT_FAILURE);
			}

			printf("Server: send reply message to client: '%s' (%d bytes)\n\n", repl_msg.reply, (int)bts);
		}
	}
}

void* cln_repl_hndl(void* args) {
	int reqs_repl_qid = 0, bts = 0;
	char* cur_dt = NULL;
	REQS_REPL_MSG* reqs_repl_msg = NULL;

	if (args == NULL) {
		perror("Server: client reply thread args is NULL");
		pthread_exit((void*)EXIT_FAILURE);
	}

	reqs_repl_qid = (int)args;

	while(1) {
		reqs_repl_msg = malloc(REQS_REPL_MSG_SIZE);
		if (reqs_repl_msg == NULL) {
			printf("Server: malloc(reqs_repl_msg)");
			pthread_exit((void*)EXIT_FAILURE);
		}

		bts = msgrcv(reqs_repl_qid, (void*)reqs_repl_msg, REQS_REPL_MSG_SIZE, REQS_MSG_TYPE, 0);
                if (bts == -1) {
			perror("Server: msgrcv(reqs)");
                       	pthread_exit((void*)EXIT_FAILURE);
                }

		printf("Server: request accepted for processing: '%s'\n", reqs_repl_msg->request);

		cur_dt = get_cur_dt("[%D][%T]");

		memset(reqs_repl_msg->reply, 0, MAX_REPL_MSG_SIZE);
		strncpy(reqs_repl_msg->reply, cur_dt, MAX_REPL_MSG_SIZE);
		reqs_repl_msg->msg_type = REPL_MSG_TYPE;

		bts = msgsnd(reqs_repl_qid, (void*)reqs_repl_msg, REQS_REPL_MSG_SIZE, 0);
		if (bts == -1) {
			perror("Server: msgsnd(reply)");
			pthread_exit((void*)EXIT_FAILURE);
		}

		printf("Server: request processed, reply: '%s'\n", reqs_repl_msg->reply);
	}
}

void make_key() {
	int rtn = 0;

	rtn = pthread_key_create(&key, destructor);
	if (rtn != 0) {
		perror("pthread_key_create");
		pthread_exit((void*)EXIT_FAILURE);
	}
}

void destructor(void* ptr) {
	free(ptr);
}

char* get_cur_dt(char* format) {
	time_t rt;
	struct tm* ti = NULL;
	char* dt = NULL;
	int rtn = 0;

	rtn = pthread_once(&key_once, make_key);
	if (rtn != 0) {
		perror("pthread_once");
		pthread_exit((void*)EXIT_FAILURE);
	}

	dt = pthread_getspecific(key);
	if (dt == NULL) {
		dt = malloc(MAX_REPL_MSG_SIZE * sizeof(char));

		rtn = pthread_setspecific(key, dt);
		if (rtn != 0) {
			perror("pthread_setspecific");
			pthread_exit((void*)EXIT_FAILURE);
		}
	}

	time(&rt);
	ti = localtime(&rt);

	strftime(dt, MAX_REPL_MSG_SIZE, format, ti);

	return dt;
}

void sys_log(char* msg, int type, int fd) {
	char *cur_dt, sys_msg[MAX_LOG_MSG_SIZE];
	cur_dt = get_cur_dt("[%D][%T]");

	switch(type) {
		case INFO:
			snprintf(sys_msg, MAX_LOG_MSG_SIZE, "%s [%s]: %s\n", cur_dt, "INFORMATION", msg);
			break;

		case WARNING:
			snprintf(sys_msg, MAX_LOG_MSG_SIZE, "%s [%s]: %s\n", cur_dt, "WARNING", msg);
			break;

		case ERROR:
			snprintf(sys_msg, MAX_LOG_MSG_SIZE, "%s [%s]: %s\n", cur_dt, "ERROR", msg);
			break;
	}

	write(fd, &sys_msg, strlen(sys_msg));
}
