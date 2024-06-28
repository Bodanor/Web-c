#include "utils.h"

char *make_path_relative(const char *path)
{
	char *rel_path;

	if (*path != '/')
		return NULL;
	rel_path = (char*)malloc(sizeof(char)*strlen(path) + 2); /* NULL terminator + '.' at the beginning */
	if (rel_path== NULL)
		return NULL;
	strcpy(rel_path, ".");
	strcat(rel_path, path);

	return rel_path;

}
