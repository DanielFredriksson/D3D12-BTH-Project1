#include "D3D12Renderer.h"

#include <d3dcompiler.h>

#include "D3D12Mesh.h"


void D3D12Renderer::getHardwareAdapter(IDXGIFactory4 * pFactory, IDXGIAdapter1 ** ppAdapter)
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

HWND D3D12Renderer::initWindow(unsigned int width, unsigned int height)
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

void D3D12Renderer::initShadersAndPipelineState()
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
	//		• Creation
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};
	//		• Specify pipeline stages
	gpsd.pRootSignature = m_rootSignature;
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(vertexBlob->GetBufferPointer());
	gpsd.VS.BytecodeLength = vertexBlob->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(pixelBlob->GetBufferPointer());
	gpsd.PS.BytecodeLength = pixelBlob->GetBufferSize();
	//		• Specify render target and depthstencil usage
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;
	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;
	//		• Specify rasterizer behaviour
	gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//		• Specify blend descriptions
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

void D3D12Renderer::initViewportAndScissorRect()
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

void D3D12Renderer::enableDebugLayer()
{
	ID3D12Debug *debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
	else {
		throw std::exception("ERROR: Failed to getDebugInterface.");
	}
}

void D3D12Renderer::initDevice()
{
	/* Factory
	The factory is created so that we can iterate through the available adapters
	and choose one which supports Direct3D 12. If no adapter is found, a 'warp adapter'
	is constructed, which is a single general purpose software rasterizer.
	*/
	if FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory))) {
		throw std::exception("ERROR: Failed to create DXGIFactory1");
	}
	// Hardware Adapter
	IDXGIAdapter1 *hardwareAdapter;
	this->getHardwareAdapter(m_factory, &hardwareAdapter);
	// Create Device
	if (FAILED(D3D12CreateDevice(
		hardwareAdapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_device)
	))) {
		throw std::exception("ERROR: Failed to create Device!");
	}
	// Release
}

void D3D12Renderer::initCommandQueue()
{
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
}

void D3D12Renderer::initSwapChain()
{
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
	if (FAILED(m_factory->CreateSwapChainForHwnd(
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
}

void D3D12Renderer::initFenceAndEventHandle()
{
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
}

void D3D12Renderer::initRenderTargets()
{
	// Descriptor Heap Description
	D3D12_DESCRIPTOR_HEAP_DESC dheapDesc = {};
	dheapDesc.NumDescriptors = this->frameCount;
	dheapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	// Create Descriptor Heap
	if (FAILED(m_device->CreateDescriptorHeap(&dheapDesc, IID_PPV_ARGS(&m_rtvHeap)))) {
		throw std::exception("ERROR: Failed to create Descriptor Heap!");
	}
	// Per Frame/swapbuffer
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	/// S T E F A N S     C O D E
	//HRESULT hr = m_device->CreateDescriptorHeap(&dheapDesc, IID_PPV_ARGS(&m_rtvHeap));

	for (int i = 0; i < this->frameCount; i++) {
		/// S T E F A N S     C O D E
		//hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		//m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, cdh);
		//cdh.ptr += m_rtvDescriptorSize;
		///------------------------------------------------------------------

		// ?
		if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])))) {
			throw std::exception("ERROR: Failed to create Render Target!");
		}
		// Create target view and increment rtvDescriptorSize
		m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, cdh);
		cdh.ptr += m_rtvDescriptorSize;
	}
}

void D3D12Renderer::initRootSignature()
{
	// Define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE dtRanges[1];
	dtRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRanges[0].NumDescriptors = 1;
	dtRanges[0].BaseShaderRegister = 0;
	dtRanges[0].RegisterSpace = 0;
	dtRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// Create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt = {};
	dt.NumDescriptorRanges = ARRAYSIZE(dtRanges);
	dt.pDescriptorRanges = dtRanges;

	// Create root parameter
	D3D12_ROOT_PARAMETER rp[1];
	rp[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rp[0].DescriptorTable = dt;
	rp[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	// Root Signature Desc
	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rp);
	rsDesc.pParameters = rp;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr;

	// Serialize root signature
	ID3DBlob* sBlob;
	D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sBlob,
		nullptr		// Possibly want to have an error blob
	);

	// Create root signature
	m_device->CreateRootSignature(
		0,
		sBlob->GetBufferPointer(),
		sBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)
	);
}

void D3D12Renderer::initConstantBuffers()
{
	// Per SwapBuffer
	for (int i = 0; i < this->frameCount; i++) {
		// Descriptor Description
		D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
		heapDescriptorDesc.NumDescriptors = 1;
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		// Create Descriptor
		if (FAILED(m_device->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&m_rtvHeap)))) {
			throw std::exception("ERROR: Failed to create Descriptor Heap!");
		}
	}

	// Heap Properties
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1;	// Used when multi-gpu
	heapProperties.VisibleNodeMask = 1;		// Used when multi-gpu
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	// Resource Description
	UINT cbSizeAligned = (sizeof(ConstantBuffer) + 255) & ~255; //???
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = cbSizeAligned;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	for (int i = 0; i < this->frameCount; i++) {
		// Committed Resource
		if (FAILED(m_device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBufferResource[i])
		))) {
			throw std::exception("ERROR: Failed to create commited resource for CBs!");
		}

		m_constantBufferResource[i]->SetName(L"cb heap");
		// Constant Buffer Description
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_constantBufferResource[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = cbSizeAligned;
		// Create Constant Buffer
		m_device->CreateConstantBufferView(&cbvDesc, m_descriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());
	}
}

void D3D12Renderer::loadPipeline()
{
#ifdef _DEBUG
	this->enableDebugLayer();
#endif

	this->initDevice();
	this->initCommandQueue();
	this->initSwapChain();
	this->initFenceAndEventHandle();
	this->initRenderTargets();
	this->initViewportAndScissorRect();
	this->initRootSignature();
	this->initShadersAndPipelineState();
	this->initConstantBuffers();

	///  -------  Create Triangle Data  -------
	Vertex triangleVertices[3] = {
		0.0f, 0.5f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		0.5f, -0.5f, 0.0f,	//v1
		0.0f, 1.0f, 0.0f,	//v1 color

		-0.5f, -0.5f, 0.0f, //v2
		0.0f, 0.0f, 1.0f	//v2 color
	};

	/* Note:
	Using upload heaps to transfer static data like vertice buffers is not recommended.
	Every time the GPU needs it, the upload heap will be marshalled over. Please read up
	on Default Heap Usage. An upload heap is used here for code simplicity and because
	there are very few vertices to actually transfer.
	*/
	// Heap Properties
	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;
	// Resource Description
	D3D12_RESOURCE_DESC rd = {};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = sizeof(triangleVertices);
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// Create Vertex Buffer
	if (FAILED(m_device->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)
	))) {
		throw std::exception("ERROR: Failed to create VertexBuffer?");
	}

	m_vertexBuffer->SetName(L"vb heap");
	// Copy data from triangleVertices to a void* and map it to the Vertex Buffer
	void* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 };
	m_vertexBuffer->Map(0, &range, &dataBegin);
	memcpy(dataBegin, triangleVertices, sizeof(triangleVertices));
	m_vertexBuffer->Unmap(0, nullptr);
	// VertexBufferView
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = sizeof(triangleVertices);
}

void D3D12Renderer::loadAssets()
{
}

void D3D12Renderer::waitForGpu()
{
	// Currently waits the entire cpu, which could do things while
	// we wait for the gpu.

	// Signal and increment the fence value
	const UINT64 fence = m_fenceValue;
	m_commandQueue->Signal(m_fence, fence);
	m_fenceValue++;
	
	// Wait until the command queue is done.
	if (m_fence->GetCompletedValue() < fence) {
		m_fence->SetEventOnCompletion(fence, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

D3D12Renderer::D3D12Renderer()
{
}

D3D12Renderer::~D3D12Renderer()
{
}

///  ------  Inherited Functions  ------ 
///  ------  Inherited Functions  ------ 
///  ------  Inherited Functions  ------ 

Material * D3D12Renderer::makeMaterial(const std::string & name)
{
	return nullptr;
}

Mesh * D3D12Renderer::makeMesh()
{
	return new D3D12Mesh();
}

VertexBuffer * D3D12Renderer::makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage)
{
	return nullptr;
}

Texture2D * D3D12Renderer::makeTexture2D()
{
	return nullptr;
}

Sampler2D * D3D12Renderer::makeSampler2D()
{
	return nullptr;
}

RenderState * D3D12Renderer::makeRenderState()
{
	return nullptr;
}

std::string D3D12Renderer::getShaderPath()
{
	return std::string();
}

std::string D3D12Renderer::getShaderExtension()
{
	return std::string();
}

ConstantBuffer * D3D12Renderer::makeConstantBuffer(std::string NAME, unsigned int location)
{
	return nullptr;
}

Technique * D3D12Renderer::makeTechnique(Material *m, RenderState *r)
{
	Technique* t = new Technique(m, r);
	return t;
}

int D3D12Renderer::initialize(unsigned int width, unsigned int height)
{
	m_wndHandle = initWindow(width, height);
	ShowWindow(m_wndHandle, 1); //Display window, move to correct place when "game loop" has been implemented
	this->loadPipeline();
	this->loadAssets();

	this->SCREEN_WIDTH = width;
	this->SCREEN_HEIGHT = height;

	return 1;
}


void D3D12Renderer::setWinTitle(const char * title)
{
	SetWindowTextA(m_wndHandle, title);
}

void D3D12Renderer::present()
{
}

int D3D12Renderer::shutdown()
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

void D3D12Renderer::setClearColor(float r, float g, float b, float a)
{
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
}

void D3D12Renderer::clearBuffer(unsigned int)
{

}

void D3D12Renderer::setRenderState(RenderState *ps)
{
}

void D3D12Renderer::submit(Mesh * mesh)
{
}

void D3D12Renderer::frame()
{

}

