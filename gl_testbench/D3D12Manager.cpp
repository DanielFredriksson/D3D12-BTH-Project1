#include "D3D12Manager.h"


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
		*wndHandle,
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

	///  -------  Render Targets  -------
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

	for (int i = 0; i < this->frameCount; i++) {
		// ?
		if (FAILED(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])))) {
			throw std::exception("ERROR: Failed to create Render Target!");
		}
		// Create target view and increment rtvDescriptorSize
		m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, cdh);
		cdh.ptr += m_rtvDescriptorSize;
	}

	///  -------  Viewport & ScissorRect  -------

	
	
	
	
	///  -------  Root Signature  -------
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

	///  -------  Shaders & Pipeline States  -------





	///  -------  Constant Buffers  -------
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
	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rd = {};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = sizeof(triangleVertices);
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

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

	void* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 };
	m_vertexBuffer->Map(0, &range, &dataBegin);
	memcpy(dataBegin, triangleVertices, sizeof(triangleVertices));
	m_vertexBuffer->Unmap(0, nullptr);

	m_vertexBuffer;


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
}

void D3D12Manager::submit(Mesh * mesh)
{
}

void D3D12Manager::frame()
{
}



