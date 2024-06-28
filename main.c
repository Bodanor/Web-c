#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#include "socket.h"
#include "http_requests.h"
#define PORT 8000

void intHandler(int signum);

void intHandler(int signum)
{
	printf("Gracefully shutting down the server...\n");
	pthread_exit(0);
}
int main(int argc, char *argv[])
{
	int server_socket;
	struct client_t *client;
	pthread_t thread_handle1;

	signal(SIGINT, intHandler);
	server_socket = Create_server(PORT);
	if (server_socket == -1)
		exit(1);
	while (1){
		client = accept_client(server_socket);
		if (client != NULL) {
			pthread_create(&thread_handle1, NULL, (void*(*)(void*))http_request_client_handler, client);
			pthread_detach(thread_handle1);
		}
	}
	close(server_socket);
	pthread_exit(0);
	
	return 0;
}
