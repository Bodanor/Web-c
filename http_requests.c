#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "http_requests.h"
#include "http_response.h"
#include "socket.h"

static int parse_path(struct http_header_t *header, char *raw_header);
static int parse_http_version(struct http_header_t *header, char *raw_header);
static int read_header(int client_socket, char **header);
static int parse_method(struct http_header_t *header, char *raw_header);
static int parse_header(struct http_header_t **header, char *raw_header);

static int parse_http_version(struct http_header_t *header, char *raw_header)
{
	char *http_version;

	http_version = strtok(NULL, "\n");
	if (http_version == NULL)
		return -2;
	if (strncmp(http_version, "HTTP", 4) !=0)
		return -2;
	
	header->http_version = (char*)malloc(sizeof(char)*strlen(http_version) + 1);
	if (header->http_version == NULL)
		return -1;

	strcpy(header->http_version, http_version);

	return 0;
}
static int parse_method(struct http_header_t *header, char *raw_header)
{
	char *method;

	method = strtok(raw_header, " ");
	if (method == NULL)
		return -2;

	if ( strncmp(method, "GET", 3) == 0 ||
	     strncmp(method, "PUT", 3) == 0 ||
	     strncmp(method, "HEAD", 4) == 0 ||
	     strncmp(method, "POST", 4) == 0 ||
	     strncmp(method, "DELETE", 6) == 0 ||
	     strncmp(method, "CONNECT", 7) == 0 ||
	     strncmp(method, "OPTIONS", 7) == 0 ||
	     strncmp(method, "TRACE", 5) == 0||
	     strncmp(method, "PATCH", 5) == 0) {

		header->method = (char*)malloc(sizeof(char)*strlen(method) + 1);
		if (header->method == NULL)
			return -1;

		strcpy(header->method, method);
	}
	else {
		return -2;
	}

	return 0;

}
static int parse_path(struct http_header_t *header, char *raw_header)
{
	char *path;

	path = strtok(NULL, " ");
	if (path == NULL) {
		return -2;
	}
	if (*path != '/')
		return -2;
	header->path = (char*)malloc(sizeof(char)*strlen(path) +1);
	if (header->path == NULL)
		return -1;
	strcpy(header->path, path);

	return 0;

}
static int read_header(int client_socket, char **header);
static int read_header(int client_socket, char **header)
{
	char *header_ptr;
	int header_end;
	size_t bytes_read;
	size_t header_total_size;
	int step;

	header_ptr = NULL;
	header_end = 0;
	header_total_size = 0;
	step = 2;

	*header = (char*)malloc(sizeof(char)*HEADER_SIZE_CHUNK);
	if (header == NULL) {
		return -1;
	}
	header_ptr = *header;

	while(header_end != 1 && (bytes_read = read(client_socket, header_ptr, HEADER_SIZE_CHUNK)) != 0) {
		if (header_total_size >= 8000) {
			free(header);
			return -2;
		}
		if (bytes_read == HEADER_SIZE_CHUNK) {
			*header = (char*)realloc(header, sizeof(char)*HEADER_SIZE_CHUNK*step);
			if (header == NULL) {
				free(header);
				return -1;
			}
			header_ptr = *header + (step *HEADER_SIZE_CHUNK) +1;
		}
		header_total_size += bytes_read;
		if (strncmp(header_ptr + bytes_read - 4, "\r\n\r\n", 4) == 0)
			header_end = 1;
	}

	*header = (char*)realloc(*header, header_total_size + 1);
	if (*header == NULL) {
		free(header);
		return -1;
	}
	(*header)[header_total_size] = '\0';

	return 0;


}
void *http_request_client_handler(void *client_structure)
{
	struct http_header_t *header;
	struct client_t *client;
	int header_error_code;
	char *raw_header;

	client	= (struct client_t*)client_structure;

	header_error_code = read_header(client->c_socket, &raw_header);
	if (header_error_code == -1) {
		/* Send a page saying that there was a server internal problem*/
	}
	else if (header_error_code == -2) {
		/* Sent an error saying that the header is too long or mal formatted */
	}
	header_error_code = parse_header(&header, raw_header);
	if (header_error_code == -1) {
		/* Send server internal error page */
	}
	else if (header_error_code == -2) {
		/* Send Malformated error page */
	}

	handle_response(header, client);
	
	free(raw_header);
	close(client->c_socket);
	destroy_client(client);
	destroy_http_structure(header);
	return NULL;
}

static int parse_header(struct http_header_t **header, char *raw_header)
{
	int parse_status;

	*header = (struct http_header_t*)malloc(sizeof(struct http_header_t));
	if (*header == NULL)
		return -1;

	/* Parse the method used */
	parse_status = parse_method(*header, raw_header);
	if (parse_status != 0)
		return parse_status;

	parse_status = parse_path(*header, raw_header);
	if (parse_status != 0)
		return parse_status;

	parse_status = parse_http_version(*header, raw_header);
	if (parse_status != 0)
		return parse_status;

	return 0;

}
void destroy_http_structure(struct http_header_t *header)
{
	if (header != NULL) {
		if (header->http_version != NULL)
			free(header->http_version);
		if (header->method != NULL)
			free(header->method);
		if (header->path != NULL)
			free(header->path);
		free(header);
	}
}
