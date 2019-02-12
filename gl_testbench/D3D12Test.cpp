#include "D3D12Test.h"
#include "Locator.h"

//make* function dependencies
#include "D3D12Mesh.h"
#include "D3D12ConstantBuffer.h"
#include "D3D12VertexBuffer.h"
#include "D3D12Material.h"
#include "D3D12RenderState.h"
#include "D3D12Technique.h"

#pragma region wndProc2
LRESULT CALLBACK wndProc2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
#pragma endregion

#pragma region initWindow
HWND D3D12Test::initWindow(unsigned int width, unsigned int height)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = wndProc2;
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
#pragma endregion

#pragma region WaitForGpu
void D3D12Test::WaitForGpu()
{
	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	//This is code implemented as such for simplicity. The cpu could for example be used
	//for other tasks to prepare the next frame while the current one is being rendered.

	//Signal and increment the fence value.
	const UINT64 fence = gFenceValue;
	ThrowIfFailed(gCommandQueue->Signal(gFence, fence));
	gFenceValue++;

	//Wait until command queue is done.
	if (gFence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(gFence->SetEventOnCompletion(fence, gEventHandle));
		WaitForSingleObject(gEventHandle, INFINITE);
	}
}
#pragma endregion

#pragma region SetResourceTransitionBarrier
void D3D12Test::SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
	D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = StateBefore;
	barrierDesc.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);
}
#pragma endregion

#pragma region CreateDirect3DDevice
void D3D12Test::CreateDirect3DDevice(HWND wndHandle)
{
#ifdef _DEBUG
	//Enable the D3D12 debug layer.
	ID3D12Debug* debugController = nullptr;
	this->enableShaderBasedValidation();

#ifdef STATIC_LINK_DEBUGSTUFF
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	SafeRelease2(debugController);
#else
	HMODULE mD3D12 = GetModuleHandle(L"D3D12.dll");
	PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
	ThrowIfFailed(f(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
	SafeRelease2(&debugController);
#endif
#endif

	//dxgi1_6 is only needed for the initialization process using the adapter.
	IDXGIFactory6*	factory = nullptr;
	IDXGIAdapter1*	adapter = nullptr;
	//First a factory is created to iterate through the adapters available.
	ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&factory)));
	for (UINT adapterIndex = 0;; ++adapterIndex)
	{
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == factory->EnumAdapters1(adapterIndex, &adapter))
		{
			break; //No more adapters to enumerate.
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, __uuidof(ID3D12Device4), nullptr)))
		{
			break;
		}

		SafeRelease2(&adapter);
	}
	if (adapter)
	{
		HRESULT hr = S_OK;
		//Create the actual device.
		if (SUCCEEDED(hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&gDevice5))))
		{
			gDevice5->SetName(L"Device");
		}

		SafeRelease2(&adapter);
	}
	else
	{
		//Create warp device if no adapter was found.
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)));
		ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&gDevice5)));
		gDevice5->SetName(L"Device");
	}

	SafeRelease2(&factory);
}
#pragma endregion

#pragma region CreateCommandInterfacesAndSwapChain
void D3D12Test::CreateCommandInterfacesAndSwapChain(HWND wndHandle)
{
	//Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC cqd = {};
	ThrowIfFailed(gDevice5->CreateCommandQueue(&cqd, IID_PPV_ARGS(&gCommandQueue)));
	gCommandQueue->SetName(L"Normal CommandQueue");

	//Create command allocator. The command allocator object corresponds
	//to the underlying allocations in which GPU commands are stored.
	ThrowIfFailed(gDevice5->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&gCommandAllocator)));
	gCommandAllocator->SetName(L"Normal CommandAllocator");

	//Create command list.
	ThrowIfFailed(gDevice5->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		gCommandAllocator,
		nullptr,
		IID_PPV_ARGS(&gCommandList4)
	));
	gCommandList4->SetName(L"Main CommandList");

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	ThrowIfFailed(gCommandList4->Close());

	IDXGIFactory5*	factory = nullptr;
	ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&factory)));

	//Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = NUM_SWAP_BUFFERS;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* swapChain1 = nullptr;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		gCommandQueue,
		wndHandle,
		&scDesc,
		nullptr,
		nullptr,
		&swapChain1
	));

	if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&gSwapChain4))))
	{
		gSwapChain4->Release();
	}

	SafeRelease2(&factory);
}
#pragma endregion

#pragma region CreateFenceAndEventHandle
void D3D12Test::CreateFenceAndEventHandle()
{
	ThrowIfFailed(gDevice5->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gFence)));
	gFence->SetName(L"Fence");
	gFenceValue = 1;
	//Create an event handle to use for GPU synchronization.
	gEventHandle = CreateEvent(0, false, false, 0);
}
#pragma endregion

#pragma region CreateRenderTargets
void D3D12Test::CreateRenderTargets()
{
	//Create descriptor heap for render target views.
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	ThrowIfFailed(gDevice5->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&gRenderTargetsHeap)));
	gRenderTargetsHeap->SetName(L"RenderTargetsHeap");

	//Create resources for the render targets.
	gRenderTargetDescriptorSize = gDevice5->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = gRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();


	//One RTV for each frame.
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++)
	{
		ThrowIfFailed(gSwapChain4->GetBuffer(n, IID_PPV_ARGS(&gRenderTargets[n])));
		gDevice5->CreateRenderTargetView(gRenderTargets[n], nullptr, cdh);
		cdh.ptr += gRenderTargetDescriptorSize;

		// Setting Name for Debugging Purposes
		std::string stringName;
		if (n == 0) {
			stringName = "RenderTarget0";
		}
		else {
			stringName = "RenderTarget1";
		}

		// std::string --> char* --> wchar_t* --> LPCWSTR
		int length = strlen(stringName.c_str());
		wchar_t* wideStringName = new wchar_t[length]; 
		std::mbstowcs(wideStringName, stringName.c_str(), length);
		LPCWSTR name = wideStringName;	
		gRenderTargets[n]->SetName(name);
		/* 
			An argument could be made for deleting the new'd 'wideStringName',
			however, since it is a pointer and L"asdf" is also a pointer i assume
			that ->setName(L"asdf") handles the destruction of L"asdf", and therefore
			also the desctrution of a given LPCWSTR
		*/
	}
}
#pragma endregion

#pragma region CreateViewportAndScissorRect
void D3D12Test::CreateViewportAndScissorRect()
{
	gViewport.TopLeftX = 0.0f;
	gViewport.TopLeftY = 0.0f;
	gViewport.Width = (float)SCREEN_WIDTH;
	gViewport.Height = (float)SCREEN_HEIGHT;
	gViewport.MinDepth = 0.0f;
	gViewport.MaxDepth = 1.0f;

	gScissorRect.left = (long)0;
	gScissorRect.right = (long)SCREEN_WIDTH;
	gScissorRect.top = (long)0;
	gScissorRect.bottom = (long)SCREEN_HEIGHT;
}
#pragma endregion

#pragma region CreateConstantBufferResources
void D3D12Test::CreateConstantBufferResources()
{
	//m_testConstantBuffer = makeConstantBuffer("test", DIFFUSE_TINT);
	//m_testConstantBuffer->setData(&gConstantBufferCPU, sizeof(ConstantBufferData), nullptr, DIFFUSE_TINT);
}
#pragma endregion

#pragma region CreateRootSignature
void D3D12Test::CreateRootSignature()
{

	/* PIRATKOPIA
	// Create root descriptors
	D3D12_ROOT_DESCRIPTOR rootDescCBV = {};
	rootDescCBV.ShaderRegister = TRANSLATION;
	rootDescCBV.RegisterSpace = 0;
	D3D12_ROOT_DESCRIPTOR rootDescCBV2 = {};
	rootDescCBV2.ShaderRegister = DIFFUSE_TINT;
	rootDescCBV2.RegisterSpace = 0;

	// Create root parameters
	D3D12_ROOT_PARAMETER rootParam[2];

	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[0].Descriptor = rootDescCBV;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParam[1].Descriptor = rootDescCBV2;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr; */

	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  dtRanges[1];
	dtRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRanges[0].NumDescriptors = 1;
	dtRanges[0].BaseShaderRegister = TRANSLATION; //register b5
	dtRanges[0].RegisterSpace = 0;
	dtRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//dtRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	//dtRanges[1].NumDescriptors = 1;
	//dtRanges[1].BaseShaderRegister = DIFFUSE_TINT; //register b6
	//dtRanges[1].RegisterSpace = 0;
	//dtRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt;
	dt.NumDescriptorRanges = ARRAYSIZE(dtRanges);
	dt.pDescriptorRanges = dtRanges;

	//create root parameter
	D3D12_ROOT_PARAMETER  rootParam[1];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = dt;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = rootSignatureFlags;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr;

	ID3DBlob* sBlob;
	ThrowIfFailed(D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sBlob,
		nullptr
	));

	ThrowIfFailed(gDevice5->CreateRootSignature(
		0,
		sBlob->GetBufferPointer(),
		sBlob->GetBufferSize(),
		IID_PPV_ARGS(&gRootSignature)
	));
	gRootSignature->SetName(L"RootSignature");

	Locator::provide(&this->gRootSignature);
	Locator::provide(&this->gDevice5);
	Locator::provide(&this->gSwapChain4);
	Locator::provide(&this->gCommandList4);
	Locator::provide(&this->gCommandAllocator);
	Locator::provide(&this->gCommandQueue);
	Locator::provide(&this->gRootSignature);
	Locator::provide(&this->gPipeLineState);
}
#pragma endregion

#pragma region CreateShadersAndPipelineState
void D3D12Test::CreateShadersAndPiplelineState()
{
	/*m_testMaterial = makeMaterial("testMaterial");
	m_testMaterial->setShader("VertexShader2.hlsl", Material::ShaderType::VS);
	m_testMaterial->setShader("PixelShader.hlsl", Material::ShaderType::PS);
	std::string errorString;
	m_testMaterial->compileMaterial(errorString);
	std::cout << errorString << "\n";

	//ID3DBlob* pixelBlob;
	//D3DCompileFromFile(
	//	L"PixelShader.hlsl", // filename
	//	nullptr,		// optional macros
	//	nullptr,		// optional include files
	//	"PS_main",		// entry point
	//	"ps_5_0",		// shader model (target)
	//	0,				// shader compile options			// here DEBUGGING OPTIONS
	//	0,				// effect compile options
	//	&pixelBlob,		// double pointer to ID3DBlob		
	//	nullptr			// pointer for Error Blob messages.
	//					// how to use the Error blob, see here
	//					// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	//);

	m_testRenderState2 = makeRenderState();
	m_testRenderState2->setWireFrame(true);

	m_testTechnique = makeTechnique(m_testMaterial, m_testRenderState);
	m_testTechnique2 = makeTechnique(m_testMaterial, m_testRenderState2);*/
}
#pragma endregion

#pragma region CreateTriangleData
void D3D12Test::CreateTriangleData()
{
	//Vertex triangleVertices[3] =
	//{
	//	-0.5f, 0.5f, 0.0f,	//v0 pos
	//	1.0f, 0.0f, 0.0f,	//v0 color

	//	0.0f, -0.5f, 0.0f,	//v1
	//	0.0f, 1.0f, 0.0f,	//v1 color

	//	-1.0f, -0.5f, 0.0f, //v2
	//	0.0f, 0.0f, 1.0f	//v2 color
	//};

	//m_testVertexBuffer = makeVertexBuffer(sizeof(triangleVertices) * 100, VertexBuffer::DATA_USAGE::STATIC);

	//for (int i = 0; i < 6; i++) {
	//	Mesh* m = makeMesh();

	//	constexpr auto numberOfPosElements = 3;
	//	size_t offset = i * sizeof(triangleVertices);
	//	m_testVertexBuffer->setData(triangleVertices, sizeof(triangleVertices), offset);
	//	m->addIAVertexBufferBinding(m_testVertexBuffer, offset, numberOfPosElements, sizeof(Vertex), 0);
	//	if (i % 2 == 0) {
	//		m->technique = m_testTechnique;
	//	}
	//	else {
	//		m->technique = m_testTechnique2;
	//	}
	//	m_meshes.push_back(m);

	//	triangleVertices[0].x += 0.1f;
	//	triangleVertices[1].x += 0.1f;
	//	triangleVertices[2].x += 0.1f;
	//}

	//
}
#pragma endregion

#pragma region Update
void D3D12Test::Update(int backBufferIndex)
{
	//Update color values in constant buffer
	for (int i = 0; i < 3; i++)
	{
		gConstantBufferCPU.colorChannel[i] += 0.0001f * (i + 1);
		if (gConstantBufferCPU.colorChannel[i] > 1)
		{
			gConstantBufferCPU.colorChannel[i] = 0;
		}
	}

	//Update GPU memory
	//m_testConstantBuffer->setData(&gConstantBufferCPU, sizeof(ConstantBufferData), nullptr, 5);

	/*for (auto m : m_meshes) {
		submit(m);
	}*/
}
#pragma endregion

#pragma region Render
void D3D12Test::Render(int backBufferIndex)
{
	/// Handle all commands and then close the commandlsit
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cdh = gRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
		
		// Reset (Open Command List)
		this->bundle.reset(gCommandList4);

		// Set Back Buffer to Render
		this->setBackBufferToRender(&cdh, gCommandList4, backBufferIndex);

			// Append Non-Bundled Commands to the commandlist
			this->recordNonBundledCommands(gCommandList4, &cdh);

			// Append Bundled Commands to the commandlist
			this->bundle.appendBundleToCommandList(gCommandList4);

		// Set Back Buffer To Display
		this->setBackBufferToDisplay(&cdh, gCommandList4, backBufferIndex);

		//Close the list to prepare it for execution.
		ThrowIfFailed(gCommandList4->Close());
	}

	/// Execute Command List & Present Frame
	{
		// Execute the command list.
		ID3D12CommandList* listsToExecute[] = { gCommandList4 };
		gCommandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

		// Present the frame.
		DXGI_PRESENT_PARAMETERS pp = {};
		ThrowIfFailed(gSwapChain4->Present1(0, 0, &pp));
	}
	
	/// Wait for GPU
	WaitForGpu();
}
#pragma endregion

#pragma region publicFuncs
//----Public functions----
D3D12Test::D3D12Test() {
	m_clearColor[0] = 0.2f;
	m_clearColor[1] = 0.2f;
	m_clearColor[2] = 0.2f;
	m_clearColor[3] = 1.0f;

	//gConstantBufferCPU.colorChannel[0] = 1.0f;

	/*m_testConstantBuffer = nullptr;
	m_testVertexBuffer = nullptr;
	m_testMaterial = nullptr;
	m_testRenderState = nullptr;
	m_testRenderState2 = nullptr;
	m_testTechnique = nullptr;
	m_testTechnique2 = nullptr;*/
}

D3D12Test::~D3D12Test() {

}

void D3D12Test::setBackBufferToRender(
	D3D12_CPU_DESCRIPTOR_HANDLE* cdh,
	ID3D12GraphicsCommandList3* commandList,
	UINT backBufferIndex
)
{
	///  --------------  OLD  --------------
	//Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(
		gCommandList4,
		gRenderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);

	//Get the handle for the current render target used as back buffer.
	cdh->ptr += gRenderTargetDescriptorSize * backBufferIndex;
}

void D3D12Test::setBackBufferToDisplay(
	D3D12_CPU_DESCRIPTOR_HANDLE* cdh,
	ID3D12GraphicsCommandList3* commandList,
	UINT backBufferIndex
)
{
	//Indicate that the back buffer will now be used to present.
	SetResourceTransitionBarrier(
		gCommandList4,
		gRenderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);
}

void D3D12Test::enableShaderBasedValidation()
{
	ID3D12Debug* pDebugController0;
	ID3D12Debug1* pDebugController1;
	
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController0)));
	ThrowIfFailed(pDebugController0->QueryInterface(IID_PPV_ARGS(&pDebugController1)));
	pDebugController1->SetEnableGPUBasedValidation(true);
}

void D3D12Test::recordNonBundledCommands(
	ID3D12GraphicsCommandList3 * commandList, 
	D3D12_CPU_DESCRIPTOR_HANDLE* cdh
)
{
	/*
	Every API-CALL that can be called by bundles does so, the API Commands
	in this function are not compatible with bundles and therefore must 
	be recorded 'normally'
	*/
	// NONBUNDLED COMMANDS 2.0
	commandList->RSSetViewports(1, &gViewport);
	commandList->RSSetScissorRects(1, &gScissorRect);
	commandList->OMSetRenderTargets(
		1,
		cdh,
		true,
		nullptr
	);
	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	commandList->ClearRenderTargetView(*cdh, clearColor, 0, nullptr);
}

#pragma endregion

#pragma region InheritedFunctions
Material * D3D12Test::makeMaterial(const std::string & name)
{
	return new D3D12Material(name);
}

Mesh * D3D12Test::makeMesh()
{
	return new D3D12Mesh();
}

VertexBuffer * D3D12Test::makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage)
{
	return new D3D12VertexBuffer(size);
}

Texture2D * D3D12Test::makeTexture2D()
{
	return nullptr;
}

Sampler2D * D3D12Test::makeSampler2D()
{
	return nullptr;
}

RenderState * D3D12Test::makeRenderState()
{
	return new D3D12RenderState();
}

std::string D3D12Test::getShaderPath()
{
	return "../assets/D3D12/";
}

std::string D3D12Test::getShaderExtension()
{
	return ".hlsl";
}

ConstantBuffer * D3D12Test::makeConstantBuffer(std::string NAME, unsigned int location)
{
	return new D3D12ConstantBuffer(NAME, location);
}

Technique * D3D12Test::makeTechnique(Material *m, RenderState *r)
{
	Technique* t = new D3D12Technique(m, r);
	return t;
}

int D3D12Test::initialize(unsigned int width, unsigned int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;

	MSG msg = { 0 };

	wndHandle = initWindow(width, height);//1. Create Window

	if (wndHandle)
	{
		CreateDirect3DDevice(wndHandle);					//2. Create Device

		CreateCommandInterfacesAndSwapChain(wndHandle);	//3. Create CommandQueue and SwapChain

		CreateFenceAndEventHandle();						//4. Create Fence and Event handle

		CreateRenderTargets();								//5. Create render targets for backbuffer

		CreateViewportAndScissorRect();						//6. Create viewport and rect

		CreateRootSignature();								//7. Create root signature

		//CreateShadersAndPiplelineState();					//8. Set up the pipeline state

		//CreateConstantBufferResources();					//9. Create constant buffer data

		//CreateTriangleData();								//10. Create vertexdata

		//this->bundle.initialize(				// Initialize Bundles
		//	m_testVertexBuffer,
		//	&gViewport,
		//	&gScissorRect,
		//	&gRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart(),
		//	m_testConstantBuffer		
		//);	

		WaitForGpu();


		ShowWindow(wndHandle, 1); //Display window
		//while (WM_QUIT != msg.message)
		//{
		//	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		//	{
		//		TranslateMessage(&msg);
		//		DispatchMessage(&msg);
		//	}
		//	else
		//	{
		//		UINT backBufferIndex = gSwapChain4->GetCurrentBackBufferIndex();

		//		Update(backBufferIndex);
		//		//Render(backBufferIndex);
		//		frame();
		//		present();
		//	}
		//}
	}

	
	//shutdown();

	return 1;
}


void D3D12Test::setWinTitle(const char * title)
{
	SetWindowTextA(wndHandle, title);
}

void D3D12Test::present()
{
	//Present the frame.
	DXGI_PRESENT_PARAMETERS pp = {};
	gSwapChain4->Present1(0, 0, &pp);

	WaitForGpu(); //Wait for GPU to finish.
				  //NOT BEST PRACTICE, only used as such for simplicity.
}

int D3D12Test::shutdown()
{
	WaitForGpu();
	CloseHandle(gEventHandle);
	SafeRelease2(&gDevice5);
	SafeRelease2(&gCommandQueue);
	SafeRelease2(&gCommandAllocator);
	SafeRelease2(&gCommandList4);
	SafeRelease2(&gSwapChain4);

	SafeRelease2(&gFence);

	SafeRelease2(&gRenderTargetsHeap);
	for (int i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		SafeRelease2(&gRenderTargets[i]);
	}

	SafeRelease2(&gRootSignature);



	/*if (m_testConstantBuffer != nullptr) {
		delete m_testConstantBuffer;
	}

	if (m_testVertexBuffer != nullptr) {
		delete m_testVertexBuffer;
	}

	if (m_testMaterial != nullptr) {
		delete m_testMaterial;
	}

	if (m_testRenderState != nullptr) {
		delete m_testRenderState;
	}

	if (m_testTechnique != nullptr) {
		delete m_testTechnique;
	}

	for (unsigned int i = 0; i < m_meshes.size(); i++) {
		delete m_meshes[i];
	}*/

	return 420;
}

void D3D12Test::setClearColor(float r, float g, float b, float a)
{
	m_clearColor[0] = r;
	m_clearColor[1] = g;
	m_clearColor[2] = b;
	m_clearColor[3] = a;
}

void D3D12Test::clearBuffer(unsigned int)
{

}

void D3D12Test::setRenderState(RenderState *ps)
{

}

void D3D12Test::submit(Mesh * mesh)
{
	drawList2[mesh->technique].push_back(mesh);
}

void D3D12Test::frame()
{
	UINT backBufferIndex = gSwapChain4->GetCurrentBackBufferIndex();

	if (m_firstFrame) {
		////Close the list to prepare it for execution.
		//gCommandList4->Close();

		////Execute the command list.
		//ID3D12CommandList* listsToExecute[] = { gCommandList4 };
		//gCommandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

		//WaitForGpu();
		m_firstFrame = false;
	}

	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	gCommandAllocator->Reset();

	gCommandList4->Reset(gCommandAllocator, nullptr);

	//Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(gCommandList4,
		gRenderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);

	//Get the handle for the current render target used as back buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = gRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += gRenderTargetDescriptorSize * backBufferIndex;

	gCommandList4->OMSetRenderTargets(1, &cdh, true, nullptr);

	gCommandList4->ClearRenderTargetView(cdh, m_clearColor, 0, nullptr);

	//Set root signature because list was reset
	gCommandList4->SetGraphicsRootSignature(gRootSignature);

	//Set necessary states.
	gCommandList4->RSSetViewports(1, &gViewport);
	gCommandList4->RSSetScissorRects(1, &gScissorRect);

	gCommandList4->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto work : drawList2) //Loop through 4 different techniques
	{
		//Enable technique
		work.first->enable(this); 

		//work.first->getMaterial()->enable();

		for (auto mesh : work.second) //Loop through all meshes that uses the "work" technique
		{
			//Bind vertex buffers
			for (auto element : mesh->geometryBuffers) {
				mesh->bindIAVertexBuffer(element.first);
			}

			//Bind cb - not yet completely implemented
			mesh->txBuffer->bind(work.first->getMaterial()); //Translation

			//Add draw command to command list
			gCommandList4->DrawInstanced(3, 1, 0, 0); //3 Vertices, 1 triangle, start with vertex 0 and triangle 0

		}
	}

	//Indicate that the back buffer will now be used to present.
	SetResourceTransitionBarrier(gCommandList4,
		gRenderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);

	//Close the list to prepare it for execution.
	gCommandList4->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { gCommandList4 };
	gCommandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	drawList2.clear();
}

#pragma endregion