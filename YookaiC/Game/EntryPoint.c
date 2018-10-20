#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "SDLEx/SDLWithPlugins.h"
#include "SDLEx/Utils/MathUtils.h"
#include "SDLEx/MathEx/MathEx.h"
#include "Constants.h"
#include "SDLEx/Vulkan/SDLExVulkan.h"

#include "SDLEx/Utils/HashMap.h"

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
	vector2_log_output(vector2_add(vector2_one(), vector2_one()), "test");

	TTF_Font * testFont = TTF_OpenFont(DEFAULT_FONT_PATH, 32);

	initialize_vulkan(window, VK_MAKE_VERSION(0, 1, 0));
	create_graphics_pipeline_f(RESOURCE_FOLDER "Shaders/default.vert.spv", RESOURCE_FOLDER "Shaders/default.frag.spv");
	int texture_id = load_texture2d(RESOURCE_FOLDER "Game/Image/Ming/Ming.png");
	for (unsigned m = 0; m < get_vk_swap_chain()->ImageCount; m++)
		bind_texture2d(m, texture_id);
	int t = 0;
	// Main Loop
	while (1) {
		t++;
		if (handle_event() == EXIT_SIGNAL)
			goto LABEL_EXIT;
		clock_t b = clock();
		unsigned imageid = sdlex_begin_frame();
		for (int i = 0; i < 200 - t / 3; i++) {
			SDL_Rect p1 = { 0, 0, 200, 400 };
			p1.x += t + i;
			sdlex_render_texture(imageid, p1);
			SDL_Rect p2 = { 200, 400, 200, 400 };
			p2.x += t + i;
			sdlex_render_texture(imageid, p2);
			SDL_Rect p3 = { 100, 200, 200, 400 };
			p3.x += t + i;
			sdlex_render_texture(imageid, p3);
		}
		sdlex_end_frame(imageid);
		SDL_Log("%d", clock() - b);
		/*SDL_SetRenderDrawColor(renderer, 83, 137, 211, 255);
		SDL_RenderClear(renderer);
		draw_texture(renderer, testTexture, new_sdl_point(250, 50));
		draw_string(renderer, testFont, "新月村Test\n换行测试", new_sdl_point(100, 100));
		SDL_RenderPresent(renderer);
		SDL_UpdateWindowSurface(window);*/
		SDL_Delay(SDL_max(16 - clock() + b, 0));
	}

LABEL_EXIT:
	vkDeviceWaitIdle(get_vk_device());
	TTF_CloseFont(testFont);
	/*SDL_DestroyTexture(testTexture);
	SDL_FreeSurface(surfaceForSoftwareRender);
	SDL_DestroyRenderer(renderer);*/
	cleanup_vulkan_pipeline();
	cleanup_vulkan();
	// SDL_DestroyWindow(window);
	cleanup_sdl();
	return 0;
}
