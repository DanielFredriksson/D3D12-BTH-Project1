#include "D3D12Test.h"
#include "Locator.h"

//make* function dependencies
#include "D3D12Mesh.h"
#include "D3D12ConstantBuffer.h"
#include "D3D12VertexBuffer.h"

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
	gCommandQueue->Signal(gFence, fence);
	gFenceValue++;

	//Wait until command queue is done.
	if (gFence->GetCompletedValue() < fence)
	{
		gFence->SetEventOnCompletion(fence, gEventHandle);
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

#ifdef STATIC_LINK_DEBUGSTUFF
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	SafeRelease2(debugController);
#else
	HMODULE mD3D12 = GetModuleHandle(L"D3D12.dll");
	PFN_D3D12_GET_DEBUG_INTERFACE f = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
	if (SUCCEEDED(f(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	SafeRelease2(&debugController);
#endif
#endif

	//dxgi1_6 is only needed for the initialization process using the adapter.
	IDXGIFactory6*	factory = nullptr;
	IDXGIAdapter1*	adapter = nullptr;
	//First a factory is created to iterate through the adapters available.
	CreateDXGIFactory(IID_PPV_ARGS(&factory));
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

		}

		SafeRelease2(&adapter);
	}
	else
	{
		//Create warp device if no adapter was found.
		factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&gDevice5));
	}

	SafeRelease2(&factory);
}
#pragma endregion

#pragma region CreateCommandInterfacesAndSwapChain
void D3D12Test::CreateCommandInterfacesAndSwapChain(HWND wndHandle)
{
	//Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC cqd = {};
	gDevice5->CreateCommandQueue(&cqd, IID_PPV_ARGS(&gCommandQueue));

	//Create command allocator. The command allocator object corresponds
	//to the underlying allocations in which GPU commands are stored.
	gDevice5->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&gCommandAllocator));

	//Create command list.
	gDevice5->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		gCommandAllocator,
		nullptr,
		IID_PPV_ARGS(&gCommandList4));

	//Command lists are created in the recording state. Since there is nothing to
	//record right now and the main loop expects it to be closed, we close it.
	gCommandList4->Close();

	IDXGIFactory5*	factory = nullptr;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));

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
	if (SUCCEEDED(factory->CreateSwapChainForHwnd(
		gCommandQueue,
		wndHandle,
		&scDesc,
		nullptr,
		nullptr,
		&swapChain1)))
	{
		if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&gSwapChain4))))
		{
			gSwapChain4->Release();
		}
	}

	SafeRelease2(&factory);
}
#pragma endregion

#pragma region CreateFenceAndEventHandle
void D3D12Test::CreateFenceAndEventHandle()
{
	gDevice5->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&gFence));
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

	HRESULT hr = gDevice5->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&gRenderTargetsHeap));

	//Create resources for the render targets.
	gRenderTargetDescriptorSize = gDevice5->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = gRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	//One RTV for each frame.
	for (UINT n = 0; n < NUM_SWAP_BUFFERS; n++)
	{
		hr = gSwapChain4->GetBuffer(n, IID_PPV_ARGS(&gRenderTargets[n]));
		gDevice5->CreateRenderTargetView(gRenderTargets[n], nullptr, cdh);
		cdh.ptr += gRenderTargetDescriptorSize;
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
	m_testConstantBuffer = makeConstantBuffer("test", 5);
	m_testConstantBuffer->setData(&gConstantBufferCPU, sizeof(ConstantBufferData), nullptr, 5);
}
#pragma endregion

#pragma region CreateRootSignature
void D3D12Test::CreateRootSignature()
{
	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  dtRanges[1];
	dtRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRanges[0].NumDescriptors = 1; //only one CB in this example
	dtRanges[0].BaseShaderRegister = 0; //register b0
	dtRanges[0].RegisterSpace = 0; //register(b0,space0);
	dtRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt;
	dt.NumDescriptorRanges = ARRAYSIZE(dtRanges);
	dt.pDescriptorRanges = dtRanges;

	//create root parameter
	D3D12_ROOT_PARAMETER  rootParam[1];
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = dt;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr;

	ID3DBlob* sBlob;
	D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sBlob,
		nullptr);

	gDevice5->CreateRootSignature(
		0,
		sBlob->GetBufferPointer(),
		sBlob->GetBufferSize(),
		IID_PPV_ARGS(&gRootSignature));

	Locator::provide(this->gRootSignature);
	Locator::provide(this->gDevice5);
	Locator::provide(this->gPipeLineState);
	Locator::provide(this->gSwapChain4);
	Locator::provide(this->gCommandList4);
}
#pragma endregion

#pragma region CreateShadersAndPipelineState
void D3D12Test::CreateShadersAndPiplelineState()
{
	///////////////////////////////////////////
//     PARAMETER #1 ('Source Data')     //
/////////////////////////////////////////

	std::string shaderDataRaw = R"(#define POSITION 0
#define NORMAL 1
#define TEXTCOORD 2
#define TRANSLATION 5
#define TRANSLATION_NAME TranslationBlock
#define DIFFUSE_TINT 6
#define DIFFUSE_TINT_NAME DiffuseColor
struct VSIn
{
	float3 pos		: POS;
	float3 color	: COLOR;
};

struct VSOut
{
	float4 pos		: SV_POSITION;
	float4 color	: COLOR;
};

cbuffer CB : register(b0)
{
	float R, G, B, A;
}

VSOut VS_main( VSIn input, uint index : SV_VertexID )
{
	VSOut output	= (VSOut)0;
	output.pos		= float4( input.pos, 1.0f );
	output.color	= float4(R, G, B, A);

	return output;
})";

	// Extra step, just in case

	//////////////////////////////////////////////////
	//     PARAMETER #2 ('Source Data LENGTH')     //
	////////////////////////////////////////////////
	SIZE_T shaderSrcDataLength = static_cast<SIZE_T>(shaderDataRaw.length());



	///////////////////////////////////////////////////////////////
	//     PARAMETER #6, #7 ('Entry Point', 'Shader Model')     //
	/////////////////////////////////////////////////////////////
	std::string entryPointString;
	std::string shaderModelString;


		entryPointString = "VS_main";
		shaderModelString = "vs_5_0";

	LPCSTR entryPoint = static_cast<LPCSTR>(entryPointString.c_str());
	LPCSTR shaderModel = static_cast<LPCSTR>(shaderModelString.c_str());



	///////////////////////////////////////////////
	//     PARAMETER #10, #11 ('Blob Data')     //
	/////////////////////////////////////////////
	ID3DBlob* VS_shaderDataBlob;
	ID3DBlob* errorDataBlob;

	if (FAILED(D3DCompile(
		shaderDataRaw.data(), // A pointer to uncompiled shader data; either ASCII HLSL code or a compiled effect.
		shaderSrcDataLength,// Length of 'shaderSrcData'
		nullptr,			// You can use this parameter for strings that specify error messages.
		nullptr,			// An array of NULL-terminated macro definitions
		nullptr,			// Optional. A pointer to an ID3DInclude for handling include files (ALREADY ADDED TO 'shaderSrcData')
		entryPoint,			// The name of the shader entry point function where shader execution begins.
		shaderModel,		// A string that specifies the shader target or set of shader features to compile against.
		0,					// Flags defined by D3D compile constants.
		0,					// Flags defined by D3D compile effect constants.
		&VS_shaderDataBlob,	// A pointer to a variable that receives a pointer to the ID3DBlob interface that you can use to access the compiled code.
		&errorDataBlob		// A pointer to a variable that receives a pointer to the ID3DBlob interface that you can use to access compiler error messages.
	)))
	{
		MessageBoxA(0, (char*)errorDataBlob->GetBufferPointer(), "", 0); // Error handling
	}

	//////// Shader Compiles //////
	//ID3DBlob* vertexBlob;
	//D3DCompileFromFile(
	//	L"VertexShader.hlsl", // filename
	//	nullptr,		// optional macros
	//	nullptr,		// optional include files
	//	"VS_main",		// entry point
	//	"vs_5_0",		// shader model (target)
	//	0,				// shader compile options			// here DEBUGGING OPTIONS
	//	0,				// effect compile options
	//	&vertexBlob,	// double pointer to ID3DBlob		
	//	nullptr			// pointer for Error Blob messages.
	//					// how to use the Error blob, see here
	//					// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	//);

		///////////////////////////////////////////
//     PARAMETER #1 ('Source Data')     //
/////////////////////////////////////////
	shaderDataRaw = R"(#define POSITION 0
#define NORMAL 1
#define TEXTCOORD 2
#define TRANSLATION 5
#define TRANSLATION_NAME TranslationBlock
#define DIFFUSE_TINT 6
#define DIFFUSE_TINT_NAME DiffuseColor
struct VSOut
{
	float4 pos		: SV_POSITION;
	float4 color	: COLOR;
};

float4 PS_main( VSOut input ) : SV_TARGET0
{
	return input.color;
})";

	//////////////////////////////////////////////////
	//     PARAMETER #2 ('Source Data LENGTH')     //
	////////////////////////////////////////////////
	shaderSrcDataLength = static_cast<SIZE_T>(shaderDataRaw.length());



	///////////////////////////////////////////////////////////////
	//     PARAMETER #6, #7 ('Entry Point', 'Shader Model')     //
	/////////////////////////////////////////////////////////////
	entryPointString = "PS_main";
	shaderModelString = "ps_5_0";

	entryPoint = static_cast<LPCSTR>(entryPointString.c_str());
	shaderModel = static_cast<LPCSTR>(shaderModelString.c_str());



	///////////////////////////////////////////////
	//     PARAMETER #10, #11 ('Blob Data')     //
	/////////////////////////////////////////////

	ID3DBlob* PS_shaderDataBlob;

	if (FAILED(D3DCompile(
		shaderDataRaw.data(), // A pointer to uncompiled shader data; either ASCII HLSL code or a compiled effect.
		shaderSrcDataLength,// Length of 'shaderSrcData'
		nullptr,			// You can use this parameter for strings that specify error messages.
		nullptr,			// An array of NULL-terminated macro definitions
		nullptr,			// Optional. A pointer to an ID3DInclude for handling include files (ALREADY ADDED TO 'shaderSrcData')
		entryPoint,			// The name of the shader entry point function where shader execution begins.
		shaderModel,		// A string that specifies the shader target or set of shader features to compile against.
		0,					// Flags defined by D3D compile constants.
		0,					// Flags defined by D3D compile effect constants.
		&PS_shaderDataBlob,	// A pointer to a variable that receives a pointer to the ID3DBlob interface that you can use to access the compiled code.
		&errorDataBlob		// A pointer to a variable that receives a pointer to the ID3DBlob interface that you can use to access compiler error messages.
	)))
	{
		MessageBoxA(0, (char*)errorDataBlob->GetBufferPointer(), "", 0); // Error handling
	}

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

	////// Input Layout //////
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR"	, 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	////// Pipline State //////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	//Specify pipeline stages:
	gpsd.pRootSignature = gRootSignature;
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(VS_shaderDataBlob->GetBufferPointer());
	gpsd.VS.BytecodeLength = VS_shaderDataBlob->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(PS_shaderDataBlob->GetBufferPointer());
	gpsd.PS.BytecodeLength = PS_shaderDataBlob->GetBufferSize();

	//Specify render target and depthstencil usage.
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;

	//Specify rasterizer behaviour.
	gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL };
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	gDevice5->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&gPipeLineState));
}
#pragma endregion

#pragma region CreateTriangleData
void D3D12Test::CreateTriangleData()
{
	Vertex triangleVertices[6] =
	{
		0.0f, 0.5f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		0.5f, -0.5f, 0.0f,	//v1
		0.0f, 1.0f, 0.0f,	//v1 color

		-0.5f, -0.5f, 0.0f, //v2
		0.0f, 0.0f, 1.0f,	//v2 color

		1.0f, 0.5f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		1.5f, -0.5f, 0.0f,	//v1
		0.0f, 1.0f, 0.0f,	//v1 color

		0.5f, -0.5f, 0.0f, //v2
		0.0f, 0.0f, 1.0f,	//v2 color
	};

	m_testVertexBuffer = makeVertexBuffer(sizeof(triangleVertices), VertexBuffer::DATA_USAGE::STATIC);
	m_testVertexBuffer->setData(triangleVertices, sizeof(triangleVertices), sizeof(Vertex));
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
	m_testConstantBuffer->setData(&gConstantBufferCPU, sizeof(ConstantBufferData), nullptr, 5);
}
#pragma endregion

#pragma region Render
void D3D12Test::Render(int backBufferIndex)
{
	// Swap Front and Back Buffers
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = gRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	this->switchSwapBuffers(&cdh, gCommandList4, backBufferIndex);

	// Bunldes?
	bool useBundles = true;

	// Record commands to command list
	if (useBundles) {
		// RESET
		gBundleCommandAllocator->Reset();
		gCommandList4->Reset(gBundleCommandAllocator, gPipeLineState);

		// NONBUNDLED COMMANDS 2.0	
		gCommandList4->SetGraphicsRootSignature(gRootSignature);
		gCommandList4->RSSetViewports(1, &gViewport);
		gCommandList4->RSSetScissorRects(1, &gScissorRect);
		gCommandList4->OMSetRenderTargets(
			1,
			&cdh,
			true,
			nullptr
		);
		m_testConstantBuffer->bind(nullptr);

		float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		gCommandList4->ClearRenderTargetView(cdh, clearColor, 0, nullptr);

		//Execute the command list.
		gCommandList4->ExecuteBundle(gBundle);

		gCommandList4->Close();
	}
	else {
		// RESET
		gCommandAllocator->Reset();
		gCommandList4->Reset(gCommandAllocator, gPipeLineState);
		// NONBUNDLED COMMANDS
		gCommandList4->SetGraphicsRootSignature(gRootSignature);
		gCommandList4->RSSetViewports(1, &gViewport);
		gCommandList4->RSSetScissorRects(1, &gScissorRect);
		gCommandList4->OMSetRenderTargets(
			1,
			&cdh,
			true,
			nullptr
		);
		m_testConstantBuffer->bind(nullptr);

		float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		gCommandList4->ClearRenderTargetView(cdh, clearColor, 0, nullptr);

		gCommandList4->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_testVertexBuffer->bind(0, 1, 0);
		gCommandList4->DrawInstanced(6, 2, 0, 0); //6 Vertices, 2 triangles, start with vertex 0 and triangle 0

		//Close the list to prepare it for execution.
		gCommandList4->Close();
	}

	// Execute the command list.
	ID3D12CommandList* listsToExecute[] = { gCommandList4 };
	gCommandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	// Present the frame.
	DXGI_PRESENT_PARAMETERS pp = {};
	gSwapChain4->Present1(0, 0, &pp);

	// Wait for GPU
	WaitForGpu();
}
#pragma endregion

#pragma region RenderBundle
void D3D12Test::RenderBundle(int backBufferIndex)
{
	// Execute Commands via Bundle
//	ID3D12CommandList* listsToExecute[] = { gBundleCommandList };
//	gCommandList4->ExecuteBundle(gBundleCommandList);	// ???

	// Present the frame
	DXGI_PRESENT_PARAMETERS pp = {};
	gSwapChain4->Present1(0, 0, &pp);

	// Wait for GPU
	WaitForGpu();
}
#pragma endregion

#pragma region publicFuncs
//----Public functions----
D3D12Test::D3D12Test() {
	m_testConstantBuffer = nullptr;
	m_testVertexBuffer = nullptr;
}

D3D12Test::~D3D12Test() {

}

void D3D12Test::initBundles()
{
	///  ------  Create Bundle Components  ------ 
	// Create Bundle Allocator
	if (FAILED(gDevice5->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_BUNDLE,
		IID_PPV_ARGS(&gBundleAllocator)
	)))
	{
		throw std::exception("NO WORKU BUNDURU.");
	}

	// Create Bundle Command Allocator
	if (FAILED(gDevice5->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&gBundleCommandAllocator)
	))) {
		throw std::exception("NO WORKU BUNDURU.");
	}

	// Create the CommandList
	if (FAILED(gDevice5->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_BUNDLE,
		gBundleAllocator,				// Where is the stack?
		nullptr,
		IID_PPV_ARGS(&gBundle)			// Where is the list?
	))) {
		throw std::exception("NO WORKU BUNDURU.");
	}

	// Populate the bundle!
	this->populateBundle();
}

void D3D12Test::populateBundle()
{
	///  ------  Record Commands ------ 
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = gRenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	/// NONBUNDLED COMMANDS
	//gBundleAllocator->Reset();
	//gBundle->Reset(gBundleAllocator, gPipeLineState);
	//gBundle->SetGraphicsRootSignature(gRootSignature);
	//gBundle->RSSetViewports(1, &gViewport);
	//gBundle->RSSetScissorRects(1, &gScissorRect);
	//gBundle->OMSetRenderTargets(
	//	1,
	//	&cdh,
	//	true,
	//	nullptr
	//);
	//float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	//gBundle->ClearRenderTargetView(cdh, clearColor, 0, nullptr);

	/// BUNDLED COMMANDS
	gBundle->SetGraphicsRootSignature(gRootSignature); // Needed?
	gBundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_testVertexBuffer->bind(0, 1, 0);
	gBundle->DrawInstanced(6, 2, 0, 0);
}

void D3D12Test::switchSwapBuffers(
	D3D12_CPU_DESCRIPTOR_HANDLE* cdh, 
	ID3D12GraphicsCommandList3* commandList,
	UINT backBufferIndex
)
{
	////Indicate that the back buffer will be used as render target.
	SetResourceTransitionBarrier(
		gCommandList4,
		gRenderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);
	//Record commands.
	//Get the handle for the current render target used as back buffer.
	cdh->ptr += gRenderTargetDescriptorSize * backBufferIndex;
	//Indicate that the back buffer will now be used to present.
	SetResourceTransitionBarrier(
		gCommandList4,
		gRenderTargets[backBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);
}

#pragma endregion

#pragma region InheritedFunctions
Material * D3D12Test::makeMaterial(const std::string & name)
{
	return nullptr;
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
	return nullptr;
}

std::string D3D12Test::getShaderPath()
{
	return std::string();
}

std::string D3D12Test::getShaderExtension()
{
	return std::string();
}

ConstantBuffer * D3D12Test::makeConstantBuffer(std::string NAME, unsigned int location)
{
	return new D3D12ConstantBuffer(NAME, location);
}

Technique * D3D12Test::makeTechnique(Material *m, RenderState *r)
{
	Technique* t = new Technique(m, r);
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

		CreateShadersAndPiplelineState();					//8. Set up the pipeline state

		CreateConstantBufferResources();					//9. Create constant buffer data

		CreateTriangleData();								//10. Create vertexdata

		initBundles();

		WaitForGpu();


		ShowWindow(wndHandle, 1); //Display window
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				UINT backBufferIndex = gSwapChain4->GetCurrentBackBufferIndex();

				Update(backBufferIndex);
				Render(backBufferIndex);
			//	RenderBundle(backBufferIndex);
			}
		}
	}

	//Wait for GPU execution to be done and then release all interfaces.
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
	SafeRelease2(&gPipeLineState);

	shutdown();

	return 1;
}


void D3D12Test::setWinTitle(const char * title)
{
	SetWindowTextA(wndHandle, title);
}

void D3D12Test::present()
{
}

int D3D12Test::shutdown()
{
	if (m_testConstantBuffer != nullptr) {
		delete m_testConstantBuffer;
	}

	if (m_testVertexBuffer != nullptr) {
		delete m_testVertexBuffer;
	}
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
}

void D3D12Test::frame()
{

}

#pragma endregion