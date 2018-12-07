#include "FileUtils.h"
#include "../SDLWithPlugins.h"

char * read_file_to_char_array(const char * filename, size_t * size_out) {
	SDL_RWops * fp = SDL_RWFromFile(filename, "rb");
	if (!fp) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Read File Error: fopen returns NULL\n");
		return NULL;
	}
	Sint64 size = SDL_RWsize(fp);
	if (size < 0 || size > SDL_MAX_SINT32) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Read File Error: ftell returns %d\n", size);
		return NULL;
	}
	size_t rsize = (size_t) size;
	char * buffer = malloc(rsize + 1);
	SDL_RWseek(fp, 0, 0);
	SDL_Log("Tried to read %d from %s, returns %d\n", rsize, filename, SDL_RWread(fp, buffer, rsize, 1));
	SDL_RWclose(fp);
	*size_out = rsize;
	buffer[rsize] = '\0';
	return buffer;
}
