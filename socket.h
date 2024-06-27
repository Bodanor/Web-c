#ifndef __SOCKET_H
#define __SOCKET_H

#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>

struct client_t{
	int c_socket;
	char *c_ip;
	char *c_port;
};

int Create_server(const int port);
void destroy_client(struct client_t *client);
struct client_t *accept_client(const int server_socket);
#endif
