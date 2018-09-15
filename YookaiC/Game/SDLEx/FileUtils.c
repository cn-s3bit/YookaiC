#include "FileUtils.h"
#include "SDLWithPlugins.h"

char * read_file_to_char_array(const char * filename, size_t * size_out) {
	FILE * fp = fopen(filename, "r");
	if (!fp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Read File Error: fopen returns NULL\n");
		return NULL;
	}
	long size = ftell(fp);
	if (size < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Read File Error: ftell returns %d\n", size);
		return NULL;
	}
	char * buffer = malloc(size);
	fseek(fp, 0, 0);
	fread(buffer, size, 1, fp);
	fclose(fp);
	*size_out = size;
	return buffer;
}