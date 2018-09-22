#ifndef SDLEX_RENDER_UTILS_H
#define SDLEX_RENDER_UTILS_H
#include "../SDLWithPlugins.h"

void draw_texture(SDL_Renderer * renderer, SDL_Texture * texture, SDL_Point position);
void draw_string(SDL_Renderer * renderer, TTF_Font * font, char * str, SDL_Point position);

#endif
