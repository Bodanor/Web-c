#ifndef __HTTP_H
#define __HTTP_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define HEADER_SIZE_CHUNK 1024
struct http_header_t {
	char *method;
	char *path;
	char *http_version;
};
void *http_thread_handler(void *client_structure);
void destroy_http_structure(struct http_header_t *header);
#endif
