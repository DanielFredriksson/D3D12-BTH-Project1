#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>	// Enables IDXGIFactory4

#include "ConstantBuffer.h"

class D3D12ConstantBuffer : public ConstantBuffer {
public:
	D3D12ConstantBuffer(std::string NAME, unsigned int location, ID3D12Device4 *device, IDXGISwapChain3 *swapChain);
	~D3D12ConstantBuffer();
	void setData(const void* data, size_t size, Material* m, unsigned int location);
	void bind(Material*);

private:
	const unsigned int m_frameCount = 2;
	ID3D12Device4 *m_device;
	//ID3D12CommandQueue *m_commandQueue;
	IDXGISwapChain3 *m_swapChain;

	ID3D12DescriptorHeap* m_descriptorHeap[2] = {};
	ID3D12Resource1* m_constantBufferResource[2] = {};

	std::string m_name;
	unsigned int m_location;


	void* m_lastMat;

	bool m_hasBeenInitialized;
};