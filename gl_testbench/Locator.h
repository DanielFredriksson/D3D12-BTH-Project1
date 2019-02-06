#pragma once

#include <SDL.h>
#include <SDL_video.h>
#include "D3D12Renderer.h"
#include <d3dcompiler.h>

class Locator {
private:
	static SDL_Window* gWindow;
	static ID3D12RootSignature* gRootSignature;
	static ID3D12Device4* gDevice;
	static ID3D12PipelineState* gPipelineState;

public:
	Locator() {}
	~Locator() {}

	// PROVIDE
	static void provide(SDL_Window* window) {
		gWindow = window;
	}
	static void provide(ID3D12RootSignature* rootSignature) {
		gRootSignature = rootSignature;
	}
	static void provide(ID3D12Device4* device) {
		gDevice = device;
	}
	static void provide(ID3D12PipelineState* pipelineState) {
		gPipelineState = pipelineState;

	}

	// GET
	static SDL_Window* getSDLWindow() {
		return gWindow;
	}
	static ID3D12RootSignature* getRootSignature() {
		return gRootSignature;
	}
	static ID3D12Device4* getDevice() {
		return gDevice;
	}
	static ID3D12PipelineState* getPipelineState() {
		return gPipelineState;
	}
};