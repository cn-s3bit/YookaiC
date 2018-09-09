#include <stdio.h>
#include "SDLEx/SDLWithPlugins.h"
#include "SDLEx/ImageUtils.h"
#include "SDLEx/RenderUtils.h"
#include "SDLEx/MathUtils.h"
#include "Constants.h"
#include "SDLEx/Vulkan/SDLExVulkan.h"

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
		SDL_Log("Working Path: %s\n", argv[0]);
	init_sdl();
	SDL_Window * window = SDL_CreateWindow("Yookai 妖召", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
	/*SDL_Surface * surfaceForSoftwareRender = SDL_GetWindowSurface(window);
	SDL_Renderer * renderer = SDL_CreateSoftwareRenderer(surfaceForSoftwareRender);

	SDL_Texture * testTexture = load_texture(renderer, RESOURCE_FOLDER "Image/Ming/Ming.png");*/
	TTF_Font * testFont = TTF_OpenFont(DEFAULT_FONT_PATH, 32);

	initialize_vulkan(window, VK_MAKE_VERSION(0, 1, 0));

	// Main Loop
	while (1) {
		if (handle_event() == EXIT_SIGNAL)
			goto LABEL_EXIT;
		/*SDL_SetRenderDrawColor(renderer, 83, 137, 211, 255);
		SDL_RenderClear(renderer);
		draw_texture(renderer, testTexture, new_sdl_point(250, 50));
		draw_string(renderer, testFont, "新月村Test\n换行测试", new_sdl_point(100, 100));
		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);*/
		SDL_Delay(16);
	}

LABEL_EXIT:
	TTF_CloseFont(testFont);
	/*SDL_DestroyTexture(testTexture);
	SDL_FreeSurface(surfaceForSoftwareRender);
	SDL_DestroyRenderer(renderer);*/
	cleanup_vulkan();
	SDL_DestroyWindow(window);
	cleanup_sdl();
	return 0;
}
