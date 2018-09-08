#ifndef SDLEX_MATH_UTILS_H
#define SDLEX_MATH_UTILS_H
#include "SDLWithPlugins.h"

inline SDL_Point new_sdl_point(int x, int y) {
	SDL_Point ret;
	ret.x = x; ret.y = y;
	return ret;
}

#endif
