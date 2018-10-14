#ifndef SDLEX_MATH_UTILS_H
#define SDLEX_MATH_UTILS_H
#include "../SDLWithPlugins.h"

#define SDLEx_clamp(x, lower, upper) SDL_max(SDL_min(x, upper), lower)

inline SDL_Point new_sdl_point(int x, int y) {
	SDL_Point ret;
	ret.x = x; ret.y = y;
	return ret;
}

inline float sdlex_pong(float x, float lower, float upper) {
	if (x > upper) x = lower + x - upper;
	return x;
}

inline void sdlex_swap_float(float * x1, float * x2) {
	register float tmp = *x1;
	*x1 = *x2;
	*x2 = tmp;
}

inline float sdlex_map_float(const float old_val, const float lower_old, const float upper_old, const float lower_new, const float upper_new) {
	return (old_val - lower_old) / (upper_old - lower_old) * (upper_new - lower_new) + lower_new;
}

#endif
