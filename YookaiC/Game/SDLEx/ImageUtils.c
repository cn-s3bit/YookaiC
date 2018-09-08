#include "ImageUtils.h"

SDL_Texture * load_texture(SDL_Renderer * targetRenderer, char * filename) {
	SDL_Surface * surface = IMG_Load(filename);
	SDL_Texture * result = SDL_CreateTextureFromSurface(targetRenderer, surface);
	SDL_FreeSurface(surface);
	return result;
}


SDL_Rect texture_frame(SDL_Texture * texture) {
	SDL_Rect result;
	result.x = result.y = 0;
	SDL_QueryTexture(texture, NULL, NULL, &result.w, &result.h);
	return result;
}
