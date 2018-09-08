#include <stdio.h>
#include "SDLEx/SDLWithPlugins.h"
#include "SDLEx/ImageUtils.h"
#include "SDLEx/RenderUtils.h"
#include "SDLEx/MathUtils.h"
#include "Constants.h"

void init_sdl(void) {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
	TTF_Init();
}

void cleanup_sdl(void) {
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int handle_event(void) {
	SDL_Event e;
	if (SDL_PollEvent(&e)) {
		if (e.type == SDL_QUIT) {
			return EXIT_SIGNAL;
		}
	}
	return NO_SIGNAL;
}

int main(int argc, char ** argv) {
	if (argc > 0)
		printf("%s", argv[0]);
	init_sdl();
	SDL_Window * window = SDL_CreateWindow("Yookai 妖召", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	SDL_Texture * testTexture = load_texture(renderer, RESOURCE_FOLDER "Image/Ming/Ming.png");
	TTF_Font * testFont = TTF_OpenFont(DEFAULT_FONT_PATH, 32);

	// Main Loop
	while (1) {
		if (handle_event() == EXIT_SIGNAL)
			goto LABEL_EXIT;
		SDL_RenderClear(renderer);
		draw_texture(renderer, testTexture, new_sdl_point(250, 50));
		draw_string(renderer, testFont, "新月村Test\n换行测试", new_sdl_point(100, 100));
		SDL_RenderPresent(renderer);
	}

LABEL_EXIT:
	TTF_CloseFont(testFont);
	SDL_DestroyTexture(testTexture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	cleanup_sdl();
	return 0;
}
