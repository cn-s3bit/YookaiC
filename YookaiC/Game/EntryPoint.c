#include <stdio.h>
#include "SDL.h"

int main(int argc, char ** argv);

int main(int argc, char ** argv) {
	if (argc > 0)
		printf("%s", argv[0]);
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window * window = SDL_CreateWindow("Yookai б§ей", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 576, SDL_WINDOW_VULKAN);
	int x = 1, y = 1;
	char ch;
	while (scanf("%c", &ch), ch != 'q') {
		SDL_SetWindowPosition(window, x++, y++);
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}