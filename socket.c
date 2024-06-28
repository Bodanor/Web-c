#include "socket.h"
#include <stdlib.h>

/**
 * @brief Create a socket.
 * 
 * @param ip Ip address to assign to the socket.
 * @param port Port nulber to assign to the socket.
 * @param results pointer to pointer to a struct addrinfo that contains all the socket info.
 * @return return the newly created socket. 
 */
static int create_socket(const char *ip, const int port, struct addrinfo **results);

/* Function to generate a socket for the server or the client */
int create_socket(const char *ip, const int port, struct addrinfo **results)
{
    int server_socket;
    struct addrinfo hints;
    char buffer[50]; /* sprintf port converter buffer */

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket error !");
        return -1;
    }
    printf("[DEBUG] Socket created 1\n");
    
    /* Reutilisable socket */
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
        perror("Setsockopt Error !");
        return -1;
    }

    /* We fill in the socket address to bind */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    sprintf(buffer, "%d", port);
    
    if (ip == NULL)
    {
        if (getaddrinfo(NULL, buffer, &hints, &(*results)) != 0)
            return -1;
    }

    else
    {
        if (getaddrinfo(ip, buffer, &hints, &(*results)) != 0) 
            return -1;
    }
        
    printf("[DEBUG] getaddrinfo successfull !\n");

    return server_socket;
}

int Create_server(int port)
{
    struct addrinfo *results;
    int server_socket;
    char hosts[NI_MAXHOST];
    char ports[NI_MAXSERV];

    results = NULL;

    results = NULL;
    server_socket = create_socket(NULL, port, &results);
    if (server_socket < 0)
        return -1;

    if (bind(server_socket, results->ai_addr, results->ai_addrlen) < 0) {
        perror("Socket error !");
        return -1;
    }
    
    getnameinfo(results->ai_addr, results->ai_addrlen, hosts, NI_MAXHOST, ports, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);
    printf("[DEBUG] Socket binded with IP : %s on PORT : %s\n", hosts, ports);

    if (listen(server_socket, SOMAXCONN) == -1) {
        perror("[DEBUG] Socket error !");
        return -1;
    }

    printf("[DEBUG] Server is now listening on IP : %s on PORT : %s\n", hosts, ports);

    freeaddrinfo(results);
    return server_socket;

}

struct client_t *accept_client(const int server_socket)
{
	struct client_t *client;
	char host[NI_MAXHOST];
	char port[NI_MAXSERV];
	struct sockaddr_in client_address;

	socklen_t client_addr_lenght;

	int new_client_socket;

	if ((new_client_socket = accept(server_socket, NULL, NULL)) == -1) {
		perror("Accept error !");
		exit(1);
	}
	client_addr_lenght = sizeof(struct sockaddr_in);
	getpeername(new_client_socket, (struct sockaddr *)&client_address, &client_addr_lenght);
	getnameinfo((struct sockaddr *)&client_address, client_addr_lenght, host, NI_MAXHOST, port, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

	printf("[DEBUG] New client {%s} connected on port : %s\n", host, port);

	client = (struct client_t*)malloc(sizeof(struct client_t));
	if (client == NULL) {
		goto alloc_error;
	}
	client->c_socket = new_client_socket;
	client->c_ip = (char *)malloc(sizeof(char)*strlen(host) + 1);
	if (client->c_ip == NULL) {
		goto alloc_error;
	}
	strcpy(client->c_ip, host);
	client->c_port = (char*)malloc(sizeof(char)*strlen(port)+1);
	if (client->c_port == NULL){
		goto alloc_error;
	}
	strcpy(client->c_port, port);
	return client;

alloc_error:
	printf("[DEBUG] Allocation error, closing connection for {%s} on port : %s\n", host, port);
	close(new_client_socket);
	destroy_client(client);
	return NULL;
}
void destroy_client(struct client_t *client)
{
	if (client != NULL){
		if (client->c_ip != NULL)
			free(client->c_ip);
		if (client->c_port != NULL)
			free(client->c_port);
		free(client);
	}

}
