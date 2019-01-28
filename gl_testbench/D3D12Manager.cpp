#include "D3D12Manager.h"
#include <d3d12.h>

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
