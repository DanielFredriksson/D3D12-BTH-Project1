#include "D3D12Manager.h"



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

LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND D3D12Manager::initWindow(unsigned int width, unsigned int height)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = wndProc;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = L"D3D12_Proj";
	if (!RegisterClassEx(&wcex))
	{
		return false;
	}

	RECT rc = { 0, 0, width, height };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	return CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		L"D3D12_Proj",
		L"Direct 3D proj",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);
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
	//HWND *wndHandle = this->getHWND();
	if (!IsWindow(m_wndHandle)) {
		throw std::exception("ERROR: Failed to fetch HWND!");
	}
	IDXGISwapChain1 *swapChain1 = nullptr;
	if (FAILED(factory->CreateSwapChainForHwnd(
		m_commandQueue,
		m_wndHandle,			// Most likely windowHandle which is wrong!!
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

D3D12Manager::D3D12Manager()
{
}

D3D12Manager::~D3D12Manager()
{
}

int D3D12Manager::initialize(unsigned int width, unsigned int height)
{
	m_wndHandle = initWindow();
	ShowWindow(m_wndHandle, 1); //Display window, move to correct place when "game loop" has been implemented
	this->loadPipeline();
	this->loadAssets();

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
	SetWindowTextA(m_wndHandle, title);
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
}

void D3D12Manager::submit(Mesh * mesh)
{
}

void D3D12Manager::frame()
{
}



