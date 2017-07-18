#include "../include/client_core.h"

#define THREADS_NUM 10000

typedef struct reqs_struct {
	int id;

	int sd_cln;
	struct sockaddr_in srv_addr;
} REQS_STRUCT;

void* send_request(void* args) {
	ssize_t bts = 0;
	char reqs_msg[MAX_REQS_MSG_SIZE], repl_msg[MAX_REPL_MSG_SIZE];
	socklen_t socket_len = 0;
	REQS_STRUCT* rs = NULL;

	if (args == NULL) {
		perror("Client: args is NULL");
		pthread_exit((void*)EXIT_FAILURE);
	}

	rs = (REQS_STRUCT*)args;

	memset(reqs_msg, 0, MAX_REQS_MSG_SIZE);
	memset(repl_msg, 0, MAX_REPL_MSG_SIZE);
	socket_len = sizeof(struct sockaddr_in);

	strncpy (reqs_msg, "get_date_time", strlen("get_date_time"));

	while(1) {
		bts = sendto(rs->sd_cln, reqs_msg, strlen(reqs_msg), 0, (struct sockaddr*)&rs->srv_addr, socket_len);
		if (bts == -1) {
			perror("Client: sendto(server)");
			pthread_exit((void*)EXIT_FAILURE);
		}

		printf("Client #%d: send request message to server: '%s' (%d bytes)\n", rs->id, reqs_msg, (int)bts);

		bts = recvfrom(rs->sd_cln, repl_msg, MAX_REPL_MSG_SIZE, 0, NULL, NULL);
		if (bts == -1) {
			perror("Client: recvfrom(server)");
			pthread_exit((void*)EXIT_FAILURE);
		}

		printf("Client #%d: receive reply message from server: '%s' (%d bytes)\n", rs->id, repl_msg, (int)bts);

		usleep(500000);
	}
}

int main() {
	int i = 0, sd_cln = 0;
	struct sockaddr_in srv_addr;
	struct hostent* srv_hst = NULL;
	socklen_t socket_len = 0;

	pthread_t reqs_tid[THREADS_NUM];
	REQS_STRUCT* reqs_struct = NULL;

	memset(&srv_addr, 0, socket_len);
	socket_len = sizeof(struct sockaddr_in);

	sd_cln = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd_cln == -1) {
		perror("Benchmark: socket(client)");

		close(sd_cln);
		exit(EXIT_FAILURE);
	}

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	srv_addr.sin_port = htons(SERVER_PORT);

	srv_hst = gethostbyaddr((char *)&srv_addr.sin_addr.s_addr, 4, AF_INET);
	printf("Benchmark: udp echo server %s:%d (hs: %s)\n", inet_ntoa(srv_addr.sin_addr), ntohs(srv_addr.sin_port),
	((srv_hst != NULL) ? srv_hst->h_name : ""));

	for(i = 0; i < THREADS_NUM; i++) {
		reqs_struct = malloc(sizeof(REQS_STRUCT));
		if (reqs_struct == NULL) {
			perror("Benchmark: malloc(reqs_struct)");
			exit(EXIT_FAILURE);
		}

		reqs_struct->id = i + 1;
		reqs_struct->sd_cln = sd_cln;
		memcpy(&reqs_struct->srv_addr, &srv_addr, socket_len);

		pthread_create(&reqs_tid[i], NULL, send_request, (void*)reqs_struct);
	}

	for(i = 0; i < THREADS_NUM; i++) {
		pthread_join(&reqs_tid[i],(void**)NULL);
	}

	exit(EXIT_SUCCESS);
}
