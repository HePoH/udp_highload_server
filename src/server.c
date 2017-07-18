#include "../include/server_core.h"

int main() {
	int i = 0, sd_srv = 0, reqs_repl_qid = 0;
	struct sockaddr_in srv_addr;
	struct hostent* srv_hst = NULL;
	pthread_t cln_repl_hndl_tid[REPL_THREAD];

	init_socket(&sd_srv, &srv_addr);
	init_queue(&reqs_repl_qid);

	sys_log("Server start", INFO, STDOUT_FILENO);
	srv_hst = gethostbyaddr((char *)&srv_addr.sin_addr.s_addr, 4, AF_INET);
	printf("Server: %s:%d (hs: %s)\n", inet_ntoa(srv_addr.sin_addr), ntohs(srv_addr.sin_port),
	((srv_hst != NULL) ? srv_hst->h_name : ""));

	for (i = 0; i < REPL_THREAD; i++){
		pthread_create(&cln_repl_hndl_tid[i], NULL, cln_repl_hndl, (void*)reqs_repl_qid);
	}

	srv_reqs_hndl(sd_srv, reqs_repl_qid);

	close(sd_srv);
	exit(EXIT_SUCCESS);
}
