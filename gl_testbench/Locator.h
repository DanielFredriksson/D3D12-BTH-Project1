#pragma once

#include <SDL.h>
#include <SDL_video.h>

class Locator {
private:
	static SDL_Window* gWindow;

public:
	Locator() {}
	~Locator() {}

	// PROVIDE
	static void provide(SDL_Window* window) {
		gWindow = window;
	}

	// GET
	static SDL_Window* getSDLWindow() {
		return gWindow;
	}
};