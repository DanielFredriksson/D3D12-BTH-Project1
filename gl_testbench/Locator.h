#pragma once

#include <SDL.h>
#include <SDL_video.h>
#include "D3D12Renderer.h"
#include <d3dcompiler.h>

#include <dxgi1_6.h>

inline void ThrowIfFailed(HRESULT hr) {
	if (FAILED(hr)) {
		// Char buffer
		char stringBuffer[64] = {};
		// Append data to buffer
		sprintf_s(stringBuffer, "HRESULT of 0x%08X", static_cast<UINT>(hr));
		// Throw!
		throw std::runtime_error(std::string(stringBuffer));
	}
}

class Locator {
private:
	static ID3D12RootSignature** gRootSignature;
	static ID3D12Device4** gDevice;
	static ID3D12PipelineState** gPipelineState;
	static IDXGISwapChain4** gSwapChain;
	static ID3D12GraphicsCommandList3** gCommandList;
	static ID3D12CommandAllocator** gCommandAllocator;

public:
	Locator() {}
	~Locator() {}

	// PROVIDE
	static void provide(ID3D12RootSignature** rootSignature) {
		gRootSignature = rootSignature;
	}
	static void provide(ID3D12Device4** device) {
		gDevice = device;
	}
	static void provide(ID3D12PipelineState** pipelineState) {
		gPipelineState = pipelineState;
	}
	static void provide(IDXGISwapChain4** swapChain) {
		gSwapChain = swapChain;
	}
	static void provide(ID3D12GraphicsCommandList3** commandList) {
		gCommandList = commandList;
	}
	static void provide(ID3D12CommandAllocator** commandAllocator) {
		gCommandAllocator = commandAllocator;
	}

	// GET
	static ID3D12RootSignature* getRootSignature() {
		return *gRootSignature;
	}
	static ID3D12Device4* getDevice() {
		return *gDevice;
	}
	static ID3D12PipelineState* getPipelineState() {
		return *gPipelineState;
	}
	static IDXGISwapChain4* getSwapChain() {
		return *gSwapChain;
	}
	static ID3D12GraphicsCommandList3* getCommandList() {
		return *gCommandList;
	}
	static ID3D12CommandAllocator* getCommandAllocator() {
		return *gCommandAllocator;
	}
};