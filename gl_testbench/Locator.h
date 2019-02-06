#pragma once

#include <SDL.h>
#include <SDL_video.h>
#include "D3D12Renderer.h"
#include <d3dcompiler.h>

class Locator {
private:
	static ID3D12RootSignature* gRootSignature;
	static ID3D12Device4* gDevice;
	static ID3D12PipelineState* gPipelineState;
	static IDXGISwapChain3* gSwapChain;
	static ID3D12GraphicsCommandList3* gCommandList;

public:
	Locator() {}
	~Locator() {}

	// PROVIDE
	static void provide(ID3D12RootSignature* rootSignature) {
		gRootSignature = rootSignature;
	}
	static void provide(ID3D12Device4* device) {
		gDevice = device;
	}
	static void provide(ID3D12PipelineState* pipelineState) {
		gPipelineState = pipelineState;
	}
	static void provide(IDXGISwapChain3* swapChain) {
		gSwapChain = swapChain;
	}
	static void provide(ID3D12GraphicsCommandList3* commandList) {
		gCommandList = commandList;
	}

	// GET
	static ID3D12RootSignature* getRootSignature() {
		return gRootSignature;
	}
	static ID3D12Device4* getDevice() {
		return gDevice;
	}
	static ID3D12PipelineState* getPipelineState() {
		return gPipelineState;
	}
	static IDXGISwapChain3* getSwapChain() {
		return gSwapChain;
	}
	static ID3D12GraphicsCommandList3* getCommandList() {
		return gCommandList;
	}
};