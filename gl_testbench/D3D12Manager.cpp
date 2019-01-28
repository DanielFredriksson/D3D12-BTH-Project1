#include "D3D12Manager.h"

#include <exception>

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Set a breakpoint on this line to catch DirectX API errors
			throw std::exception();
		}
	}
}

void D3D12Manager::getHardwareAdapter(IDXGIFactory4 * pFactory, IDXGIAdapter1 ** ppAdapter)
{
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; ; ++adapterIndex) {
		IDXGIAdapter1* pAdapter = nullptr;

		if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter)) {
			// There are no more adapters to enumerate
			break;
		}



		// Check to see if the adapter supports Direct3D 12, but don't create the actual 
		// device yet.
		if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
			*ppAdapter = pAdapter;
			return;
		}
		pAdapter->Release();
	}
}

void D3D12Manager::loadPipeline()
{
	///  -------  Enable the debug layer  -------
	ID3D12Debug *debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
	else {
		throw std::exception("ERROR: Failed to getDebugInterface.");
	}

	///  -------  Create the device  -------
	// Factory
	IDXGIFactory4 *factory;
	if FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))) {
		throw std::exception("ERROR: Failed to create DXGIFactory1");
	}
	// Hardware Adapter
	IDXGIAdapter1 *hardwareAdapter;
	this->getHardwareAdapter(factory, &hardwareAdapter);
	if (FAILED(D3D12CreateDevice(
		hardwareAdapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_device)
	))) {
		throw std::exception("ERROR: Failed to create Device!");
	}

	///  -------  Command Queue  -------
	// Command Queue Description
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags;


	// Fill out a command queue description, then create the command queue

	// Fill out a swapchain description, then create the swap chain

	// Fill out a heap description, then create a descriptor heap

	// Create the render target view

	// Create the command allocator
}

void D3D12Manager::loadAssets()
{
}

void D3D12Manager::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	DX::ThrowIfFailed(m_commandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	DX::ThrowIfFailed(m_commandList->Reset(m_commandAllocator, m_pipelineState));

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature);
	m_commandList->RSSetViewports(1, &m_viewPort);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
}

void D3D12Manager::WaitForPreviousFrame() {
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. More advanced samples 
	// illustrate how to use fences for efficient resource usage.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	DX::ThrowIfFailed(m_commandQueue->Signal(m_fence, fence));
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		DX::ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

D3D12Manager::D3D12Manager()
{
}

D3D12Manager::~D3D12Manager()
{
}

void D3D12Manager::initialize()
{
}

void D3D12Manager::render()
{
	// Populate the command list
	// Execute the command list
	// IDXGISwapChain1::Present1
	// Wait for the GPU to finish

	// Create VS
	// Create PS
	// Create PSO


	// --------------------------------
	// CODE-OBSERVED ------------------
	// --------------------------------

	/// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	/// Execute the command list
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	/// Present the frame
	ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void D3D12Manager::update()
{
}

void D3D12Manager::destroy()
{
}
