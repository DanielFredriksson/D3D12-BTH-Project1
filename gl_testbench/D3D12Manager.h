#pragma once


/// D3D12
#include <d3d12.h> 
#include <D3dx12.h>
#include <C:/Program Files (x86)/Windows Kits/10/Include/10.0.16299.0/um/d3d12.h>

#include <dxgi.h>		// DirectX Graphics Infrastructure
#include <dxgi1_4.h>	// Enables IDXGIFactory4

/// DEBUGGING
#include <exception>


class D3D12Manager {
private:
	static const UINT frameCount = 2;

	// Pipeline Objects
	D3D12_VIEWPORT m_viewPort;
	D3D12_RECT m_scissorRect;
	IDXGISwapChain3 *m_swapChain;
	ID3D12Device *m_device;

	ID3D12Resource *m_renderTargets[frameCount];

	ID3D12CommandAllocator *m_commandAllocator;
	ID3D12CommandQueue *m_commandQueue;

	ID3D12RootSignature *m_rootSignature;
	ID3D12DescriptorHeap *m_rtvHeap;
	ID3D12PipelineState *m_pipelineState;
	ID3D12GraphicsCommandList *m_commandList;
	UINT m_rtvDescriptorSize;

	// App Resources
	ID3D12Resource *m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	// Synchronization Objects
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ID3D12Fence *m_fence;
	UINT64 m_fenceValue;

	void getHardwareAdapter(IDXGIFactory4 * pFactory, IDXGIAdapter1 ** ppAdapter);

	void loadPipeline();
	void loadAssets();

	// Render-related functions
	void PopulateCommandList();

public:
	D3D12Manager();
	~D3D12Manager();

	void initialize();
	void render();
	void update();
	void destroy();
};

