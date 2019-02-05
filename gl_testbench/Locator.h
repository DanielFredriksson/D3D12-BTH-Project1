#pragma once

#include <SDL.h>
#include <SDL_video.h>
#include "D3D12Renderer.h"
#include <d3dcompiler.h>

class Locator {
private:
	static SDL_Window* gWindow;
	static ID3D12RootSignature* gRootSignature;

public:
	Locator() {}
	~Locator() {}

	// PROVIDE
	static void provide(SDL_Window* window) {
		gWindow = window;
	}
	static void provide(ID3D12RootSignature* rootSignature) {
		gRootSignature;
	}

	// GET
	static SDL_Window* getSDLWindow() {
		return gWindow;
	}
	static ID3D12RootSignature* getRootSignature() {
		return gRootSignature;
	}
};