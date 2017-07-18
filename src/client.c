#include "../include/client_core.h"

int main() {
	int sd_cln = 0;
	struct sockaddr_in srv_addr;
	struct hostent* srv_hst = NULL;
	char msg[MAX_REQS_MSG_SIZE];
	socklen_t socket_len = 0;
	ssize_t bts = 0;

	socket_len = sizeof(struct sockaddr_in);
	memset(&srv_addr, 0, socket_len);
	memset(msg, 0, MAX_REQS_MSG_SIZE);

	sd_cln = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd_cln == -1) {
		perror("Client: socket(client)");

		close(sd_cln);
		exit(EXIT_FAILURE);
	}

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	srv_addr.sin_port = htons(SERVER_PORT);

	srv_hst = gethostbyaddr((char *)&srv_addr.sin_addr.s_addr, 4, AF_INET);
	printf("Client: udp echo server %s:%d (hs: %s)\n", inet_ntoa(srv_addr.sin_addr), ntohs(srv_addr.sin_port),
	((srv_hst != NULL) ? srv_hst->h_name : ""));

	while(1) {
		memset(msg, 0, MAX_REQS_MSG_SIZE);

		printf("Input you message: ");
		if (fgets(msg, MAX_REQS_MSG_SIZE, stdin) == NULL) {
			perror("Client: fgets(msg)");

			close(sd_cln);
			exit(EXIT_FAILURE);
		}

		msg[strlen(msg) - 1] = '\0';

		bts = sendto(sd_cln, msg, strlen(msg), 0, (struct sockaddr*)&srv_addr, socket_len);
		if (bts == -1) {
			perror("Client: sendto(server)");

			close(sd_cln);
			exit(EXIT_FAILURE);
		}

		printf("Client: send message to server: '%s' (%d bytes)\n", msg, (int)bts);

		if(!strcmp(msg, "exit")){
			printf("Client: main thread bye\n");
			exit(EXIT_SUCCESS);
		}

		memset(msg, 0, MAX_REQS_MSG_SIZE);

		bts = recvfrom(sd_cln, msg, MAX_REQS_MSG_SIZE, 0, NULL, NULL);
		if (bts == -1) {
			perror("Client: recvfrom(server)");

			close(sd_cln);
			exit(EXIT_FAILURE);
		}

		printf("Client: receive message from server: '%s' (%d bytes)\n\n", msg, (int)bts);
	}

	exit(EXIT_SUCCESS);
}
