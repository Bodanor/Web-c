#ifndef __HTTP_RESPONSE_H
#define __HTTP_RESPONSE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "http_requests.h"
#include "socket.h"

#define CONTENT_TYPE_STR "Content-type: "
#define CONTENT_LENGTH "Content-Length: "

#define DEFAULT_500_PATH "/defaults/500.html"
#define DEFAULT_404_PATH "/defaults/404.html"
#define DEFAULT_PAGE "/index.html"

struct http_response_t {
	/* Status line */
	char *http_version;
	char *status_code;
	char *status_phrase;

	/* We could also add some additional header fields but keep it simple at the moment */
	
	/* Content specs */
	char *content_type;
	char *content_length;

	/* Actual data */

	uint8_t *data;
	size_t data_length_in_bytes;

	size_t total_response_length;

};

void handle_response(struct http_header_t *request, struct client_t *client);
struct http_response_t *alloc_response(void);
void destroy_response(struct http_response_t *response);
void send_internal_error_page(struct client_t *client, struct http_header_t *request);
void send_not_found_error_page(struct client_t *client, struct http_header_t *request);
#endif
