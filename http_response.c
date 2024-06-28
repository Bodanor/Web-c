#include "http_response.h"
#include "socket.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char *build_raw_response(struct http_response_t *response);
static int response_add_status_line(struct http_response_t *response, char *version, char *code, char *phrase);
static int response_add_content(struct http_response_t *response, struct http_header_t *request, struct client_t *client); 
static int response_add_content_length(struct http_response_t *response, size_t content_length);
static int response_find_content_type(struct http_response_t *response, char *file);
static int response_find_content_type(struct http_response_t *response, char *file)
{
	/* Allocate the biggest content-type possible */
	response->content_type = (char*)malloc(sizeof(char)*100);
	if (response->content_type == NULL)
		return -1;
	
	strcpy(response->content_type, CONTENT_TYPE_STR);

	if (strstr(file, ".html") != NULL) {
		strcat(response->content_type, "text/html;");
		strcat(response->content_type, "\r\n");
	}
	else {
		return -2;
	}
	
	/* Reallocate the actual used bytes for the content-type fiels */
	response->content_type = (char*)realloc(response->content_type, strlen(response->content_type) + 1);
	if (response->content_type == NULL)
		return -1;

	return 0;
}
static int response_add_content_length(struct http_response_t *response, size_t content_length)
{
	char content_str[32];

	sprintf(content_str, "%zu", content_length);

	response->content_length = (char*)malloc(sizeof(char) *strlen(content_str) + strlen(CONTENT_LENGTH) +3);
	if (response->content_length == NULL)
		return -1;

	strcpy(response->content_length, CONTENT_LENGTH);
	strcat(response->content_length, content_str);
	strcat(response->content_length, "\r\n");

	return 0;

}
static int response_add_status_line(struct http_response_t *response, char *version, char *code, char *phrase)
{
	response->http_version = (char*)malloc(sizeof(char)*strlen(version) + 1);
	if (response->http_version == NULL) {
		return -1;
	}
	strcpy(response->http_version, version);

	response->status_code = (char*)malloc(sizeof(char)*strlen(code) + 1);
	if (response->status_code == NULL) {
		return -1;
	}
	strcpy(response->status_code, code);
	
	response->status_phrase = (char*)malloc(sizeof(char)*strlen(phrase) + 1);
	if (response->status_phrase == NULL){
		return -1;
	}
	strcpy(response->status_phrase, phrase);

	return 0;

}
static int response_add_content(struct http_response_t *response, struct http_header_t *request, struct client_t *client)
{
	int content_status;
	FILE *f_ptr;
	size_t data_length;
	size_t bytes_read;
	char *rel_path;
	
	/* Find the actual content-type if possible */
	content_status = response_find_content_type(response, request->path);
	if (content_status != 0)
		return content_status;
	
	rel_path = make_path_relative(request->path);
	if (rel_path == NULL)
		return -1;

	f_ptr = fopen(rel_path, "rb");
	if (f_ptr == NULL) {
		printf("[DEBUG] Client {%s} on port %s -- 404 Not found. Reason : %s not found !\n", client->c_ip, client->c_port, request->path);
		return -2;

	}
	/* Get the data length */
	fseek(f_ptr, 0, SEEK_END);
	data_length = ftell(f_ptr);

	response->data_length_in_bytes = data_length;
	response->total_response_length += data_length +2; /* Add the end of body signature */
	
	/* Reset the file pointer to point to the beginning */
	fseek(f_ptr, 0, SEEK_SET);
	
	response->data = (uint8_t*)malloc(sizeof(uint8_t) * data_length);
	if (response->data == NULL)
		goto error_cleanup;

	bytes_read = fread(response->data, data_length, 1, f_ptr);
	if (bytes_read != 1)
		goto error_cleanup;

	content_status = response_add_content_length(response, data_length);
	if (content_status != 0)
		goto error_cleanup;
	
	free(rel_path);
	fclose(f_ptr);
	return 0;

error_cleanup:
	free(rel_path);
	fclose(f_ptr);
	return -1;
}
static char *build_raw_response(struct http_response_t *response)
{
	char *raw_response;
	size_t header_length;

	raw_response = (char*)malloc(sizeof(char)*strlen(response->http_version) + strlen(response->status_code) + strlen(response->status_phrase) + 3);
	if (raw_response == NULL)
		return NULL;
	strcpy(raw_response, response->http_version);
	strcat(raw_response, response->status_code);
	strcat(raw_response, response->status_phrase);
	strcat(raw_response, "\r\n");

	raw_response = (char*)realloc(raw_response, sizeof(char)*strlen(raw_response) + strlen(response->content_type) + 3);
	if (raw_response == NULL)
		goto error_cleanup;

	strcat(raw_response, response->content_type);

	raw_response = (char*)realloc(raw_response,sizeof(char)*strlen(raw_response) + strlen(response->content_length) + 3);
	if (raw_response == NULL)
		goto error_cleanup;

	strcat(raw_response, response->content_length);

	header_length = strlen(raw_response);
	response->total_response_length += header_length;

	raw_response = (char*)realloc(raw_response, sizeof(char)*strlen(raw_response) + response->data_length_in_bytes + 2);
	if (raw_response == NULL)
		goto error_cleanup;
	
	memcpy(raw_response + strlen(raw_response), response->data, response->data_length_in_bytes);

	memcpy(raw_response + header_length + response->data_length_in_bytes, "\r\n", 2);

	return raw_response;
error_cleanup:
	free(raw_response);
	return NULL;

}
struct http_response_t *alloc_response(void)
{
	struct http_response_t *temp;

	temp = (struct http_response_t*)malloc(sizeof(struct http_response_t));
	if (temp == NULL)
		return NULL;

	temp->http_version = NULL;
	temp->status_code = NULL;
	temp->status_phrase = NULL;
	temp->content_type = NULL;
	temp->content_length = NULL;
	temp->data = NULL;

	temp->data_length_in_bytes = 0;
	temp->total_response_length = 0;

	return temp;

}
void destroy_response(struct http_response_t *response)
{
	if (response != NULL) {
		if (response->http_version)
			free(response->http_version);
		if (response->status_code)
			free(response->status_code);
		if (response->status_phrase)
			free(response->status_phrase);
		if (response->content_length)
			free(response->content_length);
		if (response->content_type)
			free(response->content_type);
		if (response->data)
			free(response->data);
		free(response);
	}

}
void handle_response(struct http_header_t *request, struct client_t *client)
{
	struct http_response_t *response;
	int response_status;
	char *raw_response;
	
	raw_response = NULL;
	response = alloc_response();

	if (response == NULL) {
		/* Send error message */
		goto error_cleanup;
	}
	response_status = response_add_content(response, request, client);
	/* Success */
	if (response_status == 0) {
		response_status = response_add_status_line(response,"HTTP/1.1 ", "200 ", "OK");
	}
	else {
		/* error */
		goto error_cleanup;

	}

	raw_response = build_raw_response(response);
	if (raw_response == NULL) {
		/* Send error message */
		goto error_cleanup;
	}

	write(client->c_socket, raw_response, response->total_response_length);

error_cleanup:
	if (raw_response != NULL)
		free(raw_response);
	destroy_response(response);
	return;
}
