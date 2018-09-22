#ifndef SDLEX_IMAGE_UTILS_H
#define SDLEX_IMAGE_UTILS_H
#include "../SDLWithPlugins.h"

SDL_Texture * load_texture(SDL_Renderer * targetRenderer, char * filename);
SDL_Rect texture_frame(SDL_Texture * texture);

#endif
