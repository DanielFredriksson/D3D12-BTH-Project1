#include "D3D12Manager.h"

#include <exception>
#include <d3dcompiler.h>

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

struct CD3DX12_RESOURCE_BARRIER : public D3D12_RESOURCE_BARRIER {
	CD3DX12_RESOURCE_BARRIER();
	explicit CD3DX12_RESOURCE_BARRIER(const D3D12_RESOURCE_BARRIER &o);
	CD3DX12_RESOURCE_BARRIER static inline Transition(ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);
	CD3DX12_RESOURCE_BARRIER static inline Aliasing(ID3D12Resource* pResourceBefore, ID3D12Resource* pResourceAfter);
	CD3DX12_RESOURCE_BARRIER static inline UAV(ID3D12Resource* pResource);
	operator const D3D12_RESOURCE_BARRIER&() const;
};

struct CD3DX12_DEFAULT {};
extern const DECLSPEC_SELECTANY CD3DX12_DEFAULT D3D12_DEFAULT;

struct CD3DX12_CPU_DESCRIPTOR_HANDLE : public D3D12_CPU_DESCRIPTOR_HANDLE {
	CD3DX12_CPU_DESCRIPTOR_HANDLE();
	explicit CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE &o);
	CD3DX12_CPU_DESCRIPTOR_HANDLE(CD3DX12_DEFAULT);
	CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE &other, INT offsetScaledByIncrementSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE(const D3D12_CPU_DESCRIPTOR_HANDLE &other, INT offsetInDescriptors, UINT descriptorIncrementSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE&  Offset(INT offsetInDescriptors, UINT descriptorIncrementSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE&  Offset(INT offsetScaledByIncrementSize);
	bool                            operator==(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) const;
	bool                            operator!=(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE& other) const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE & operator=(const D3D12_CPU_DESCRIPTOR_HANDLE &other);
	void                            inline InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetScaledByIncrementSize);
	void                            inline InitOffsetted(_In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize);
	void                            static inline InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetScaledByIncrementSize);
	void                            static inline InitOffsetted(_Out_ D3D12_CPU_DESCRIPTOR_HANDLE &handle, _In_ const D3D12_CPU_DESCRIPTOR_HANDLE &base, INT offsetInDescriptors, UINT descriptorIncrementSize);
};

/// GETHWND TESTING
#include <string>
#include <iostream>
#include <SDL.h>		// Used for retrieving HWND from SDL	
#include <SDL_syswm.h>	//
#include "Locator.h"

HWND l_HWND;
//
//BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam)
//{
//	// Is related to minimization of windows?
//	//if (!IsIconic(hWnd)) {
//	//	return HRESULT(true);
//	//}
//
//	// Modded Stuff
//	LPDWORD lpWord = NULL;
//	DWORD thisWindowID, currentWindowID;
//	currentWindowID = GetWindowThreadProcessId(hWnd, lpWord);
//	thisWindowID = GetCurrentProcessId();
//
//	if (currentWindowID == thisWindowID) {
//		// We've found the HWND!
//		l_HWND = hWnd;
//		return true;
//	}
//
//
//	//int length = GetWindowTextLength(hWnd);
//	//if (length == 0) {
//	//	return HRESULT(true);
//	//}
//	//TCHAR* buffer;
//	//int bufferSize = length + 1;
//	//buffer = new TCHAR[bufferSize];
//	//memset(buffer, 0, (bufferSize) * sizeof(TCHAR));
//
//	//GetWindowText(hWnd, buffer, bufferSize);
//	//std::string windowTitle = std::string((char*)buffer);
//
//	//delete[] buffer;
//
//	//std::cout << hWnd << ": " << windowTitle << std::endl;
//
//	return 0;
//}
//

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
		SafeRelease(&pAdapter);
	}
}

HWND *D3D12Manager::getHWND()
{
	// Fetch HWND from the SDL_Window via Locator
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(Locator::getSDLWindow(), &wmInfo);
	
	return &wmInfo.info.win.window;
}

void D3D12Manager::loadPipeline()
{
	///  -------  Enable the debug layer  -------
#ifdef _DEBUG
	ID3D12Debug *debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
	else {
		throw std::exception("ERROR: Failed to getDebugInterface.");
	}
#endif

	///  -------  Create the device  -------
	/* Factory
	The factory is created so that we can iterate through the available adapters
	and choose one which supports Direct3D 12. If no adapter is found, a 'warp adapter'
	is constructed, which is a single general purpose software rasterizer.
	*/ 
	IDXGIFactory4 *factory;
	if FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))) {
		throw std::exception("ERROR: Failed to create DXGIFactory1");
	}
	// Hardware Adapter
	IDXGIAdapter1 *hardwareAdapter;
	this->getHardwareAdapter(factory, &hardwareAdapter);
	// Create Device
	if (FAILED(D3D12CreateDevice(
		hardwareAdapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_device)
	))) {
		throw std::exception("ERROR: Failed to create Device!");
	}
	// Release


	///  -------  Command Queue  -------
	// Command Queue Description
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	// Create Command Queue
	if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)))) {
		throw std::exception("ERROR: Failed to create Command Queue!");
	}
	// Create Command Allocator
	if (FAILED(m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_commandAllocator)
	))) {
		throw std::exception("ERROR: Failed to create Command Allocator!");
	}
	// Create Command List
	if (FAILED(m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator,
		nullptr,
		IID_PPV_ARGS(&m_commandList)
	))) {
		throw std::exception("ERROR: Failed to create Command List!");
	}
	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	m_commandList->Close();


	///  -------  Swap Chain  -------
	// Swap Chain Description
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = this->frameCount;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = 0;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	/* Create Swap Chain
	- Why do we create using a swapchain1 then queryinterface to our swapchain3?
	*/
	HWND *wndHandle = this->getHWND();
	if (!IsWindow(*wndHandle)) {
		throw std::exception("ERROR: Failed to fetch HWND!");
	}
	IDXGISwapChain1 *swapChain1 = nullptr;
	if (FAILED(factory->CreateSwapChainForHwnd(
		m_commandQueue,
		*wndHandle,			// Most likely windowHandle which is wrong!!
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1
	))) {
		throw std::exception("ERROR: Failed to create Swap Chain!");
	} 
	else {
		if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain)))) {
			SafeRelease(&m_swapChain);
		}
	}

	///  -------  Fence & Event Handle-------
	// Create Fence
	if (FAILED(m_device->CreateFence(
		0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)
	))) {
		throw std::exception("ERROR: Failed to create Swap Chain!");
	}
	// Create Event Handle
	else {
		m_fenceValue = 1;
		m_fenceEvent = CreateEvent(0, false, false, 0); //Create an event handle to use for GPU synchronization.
	}

	///  -------  Descriptor Heap  -------
	D3D12_DESCRIPTOR_HEAP_DESC dheapDesc = {};
	dheapDesc.NumDescriptors = this->frameCount;
	dheapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	//if (FAILED(factory->MakeWindowAssociation(Win32Application::GetHwnd(), DXGI_MWA_NO_ALT_ENTER))) {

	//}
	

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

	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);

	// Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	DX::ThrowIfFailed(m_commandList->Close());
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

void D3D12Manager::initViewportAndScissorRect()
{
	m_viewPort.TopLeftX = 0.0f;
	m_viewPort.TopLeftY = 0.0f;
	m_viewPort.Width = (float)SCREEN_WIDTH;
	m_viewPort.Height = (float)SCREEN_HEIGHT;
	m_viewPort.MinDepth = 0.0f;
	m_viewPort.MaxDepth = 1.0f;

	m_scissorRect.left = (long)0;
	m_scissorRect.right = (long)SCREEN_WIDTH;
	m_scissorRect.top = (long)0;
	m_scissorRect.bottom = (long)SCREEN_HEIGHT;
}

void D3D12Manager::initShadersAndPipelineState()
{
	//////////////////////////
	///// VERTEX SHADER /////
	////////////////////////
	ID3DBlob* vertexBlob;
	D3DCompileFromFile(
		L"VertexShader.hlsl",		// Name
		nullptr,					// Macros (optional)
		nullptr,					// Include Files (optional)
		"VS_main",					// Entry Point
		"vs_5_0",					// Shader Model (target)
		0,							// Shader Compile Options (debugging)
		0,							// Effect Compile Options
		&vertexBlob,				// Double Pointer to ID3DBlob
		nullptr						// Pointer for Error Blob-messages
	);

	//////////////////////////
	///// PIXEL SHADER //////
	////////////////////////
	ID3DBlob* pixelBlob;
	D3DCompileFromFile(
		L"PixelShader.hlsl",		// Name
		nullptr,					// Macros (optional)
		nullptr,					// Include Files (optional)
		"PS_main",					// Entry Point
		"ps_5_0",					// Shade Model (target)
		0,							// Shader Compile Options
		0,							// Effect Compile Options
		&pixelBlob,					// Double Pointer to ID3DBlob
		nullptr						// Pointer for Error Blob-messages
	);

	// Input Layout
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	// Pipeline State:
	//		� Creation
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};
	//		� Specify pipeline stages
	gpsd.pRootSignature = m_rootSignature;
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(vertexBlob->GetBufferPointer());
	gpsd.VS.BytecodeLength = vertexBlob->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(pixelBlob->GetBufferPointer());
	gpsd.PS.BytecodeLength = pixelBlob->GetBufferSize();
	//		� Specify render target and depthstencil usage
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;
	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;
	//		� Specify rasterizer behaviour
	gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//		� Specify blend descriptions
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
	};
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	m_device->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&m_pipelineState));
}

D3D12Manager::D3D12Manager()
{
}

D3D12Manager::~D3D12Manager()
{
}

int D3D12Manager::initialize(unsigned int width, unsigned int height)
{
	this->loadPipeline();
	this->loadAssets();
	this->getHWND();

	return 1;
}


///  ------  Inherited Functions  ------ 
///  ------  Inherited Functions  ------ 
///  ------  Inherited Functions  ------ 

Material * D3D12Manager::makeMaterial(const std::string & name)
{
	return nullptr;
}

Mesh * D3D12Manager::makeMesh()
{
	return nullptr;
}

VertexBuffer * D3D12Manager::makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage)
{
	return nullptr;
}

Texture2D * D3D12Manager::makeTexture2D()
{
	return nullptr;
}

Sampler2D * D3D12Manager::makeSampler2D()
{
	return nullptr;
}

RenderState * D3D12Manager::makeRenderState()
{
	return nullptr;
}

std::string D3D12Manager::getShaderPath()
{
	return std::string();
}

std::string D3D12Manager::getShaderExtension()
{
	return std::string();
}

ConstantBuffer * D3D12Manager::makeConstantBuffer(std::string NAME, unsigned int location)
{
	return nullptr;
}

Technique * D3D12Manager::makeTechnique(Material *, RenderState *)
{
	return nullptr;
}

void D3D12Manager::setWinTitle(const char * title)
{
}

void D3D12Manager::present()
{
}

int D3D12Manager::shutdown()
{
	// Possibly might have to need to wait for GPU or things to finish before cleaning

	// Possibly might have to be done in some certain order...
	SafeRelease(&m_swapChain);
	SafeRelease(&m_device);
	SafeRelease(&m_commandAllocator);
	SafeRelease(&m_commandQueue);
	SafeRelease(&m_commandList);
	SafeRelease(&m_rootSignature);
	SafeRelease(&m_rtvHeap);
	SafeRelease(&m_pipelineState);
	SafeRelease(&m_vertexBuffer);
	SafeRelease(&m_commandAllocator);

	return 420;
}

void D3D12Manager::setClearColor(float r, float g, float b, float a)
{
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
}

void D3D12Manager::clearBuffer(unsigned int)
{

}

void D3D12Manager::setRenderState(RenderState *ps)
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
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	/// Present the frame
	DX::ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void D3D12Manager::submit(Mesh * mesh)
{
}

void D3D12Manager::frame()
{
}



