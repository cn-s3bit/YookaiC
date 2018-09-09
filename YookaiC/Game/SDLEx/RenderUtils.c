#include "RenderUtils.h"
#include "ImageUtils.h"
#include <stdio.h>

void draw_texture(SDL_Renderer * renderer, SDL_Texture * texture, SDL_Point position) {
	SDL_Rect drawRect = texture_frame(texture);
	drawRect.x = position.x;
	drawRect.y = position.y;
	SDL_RenderCopy(renderer, texture, NULL, &drawRect);
}
void draw_string(SDL_Renderer * renderer, TTF_Font * font, char * str, SDL_Point position) {
	SDL_Color defaultColor = { .a = 255u,.b = 255u,.g = 255u,.r = 255u };
	int textWidth = 0;
	TTF_SizeUTF8(font, str, &textWidth, NULL);
	
	SDL_Surface * textSurface = TTF_RenderUTF8_Blended_Wrapped(font, str, defaultColor, SDL_min(textWidth, 1024u));
	SDL_Texture * tempTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_FreeSurface(textSurface);
	draw_texture(renderer, tempTexture, position);
	SDL_DestroyTexture(tempTexture);
}
