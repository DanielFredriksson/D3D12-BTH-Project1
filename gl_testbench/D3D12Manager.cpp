#include "D3D12Manager.h"


/// GETHWND TESTING
#include <string>
#include <iostream>

BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam)
{
	// Is related to minimization of windows?
	if (!IsIconic(hWnd)) {
		return HRESULT(true);
	}

	int length = GetWindowTextLength(hWnd);
	if (length == 0) {
		return HRESULT(true);
	}
	TCHAR* buffer;
	int bufferSize = length + 1;
	buffer = new TCHAR[bufferSize];
	memset(buffer, 0, (bufferSize) * sizeof(TCHAR));

	GetWindowText(hWnd, buffer, bufferSize);
	std::string windowTitle = std::string((char*)buffer);

	delete[] buffer;

	std::cout << hWnd << ": " << windowTitle << std::endl;


	// Modded Stuff
	LPDWORD lpWord = NULL;
	DWORD thisWindowID, currentWindowID;

	currentWindowID = GetWindowThreadProcessId(hWnd, lpWord);
	thisWindowID = GetCurrentProcessId();

	if (currentWindowID == thisWindowID) {
		// We've found the HWND!
		int asdf = 3;
	}

	return 0;
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

HRESULT D3D12Manager::getHWND()
{
	std::cout << "Enumerating Windows..." << std::endl;
	BOOL enumeratingWindowsSucceeded = EnumWindows(enumWindowsProc, NULL);





	// Walk through the existing windows with EnumWindows
	WNDENUMPROC lpEnumFunc;
	LPARAM lParam;
	EnumWindows(lpEnumFunc, lParam);
	
	// Check their ownership iwth GetWindowThreadProcessID

	// Compare the returned process ID with the one returned by GetCurrentProcessId.

	// Alternatively, the handle can be saved when the window is created rather than fetched later on,
	// However i have no clue on where we do this.

	return HRESULT(true);
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
	// Create Device
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
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	// Create Command Queue
	if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)))) {
		throw std::exception("ERROR: Failed to create Command Queue!");
	}

	///  -------  Swap Chain  -------
	// Swap Chain Description
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = this->frameCount;
//	swapChainDesc.BufferDesc.Width = m_width;
//	swapChainDesc.BufferDesc.Height = m_height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//	swapChainDesc.OutputWindow = Win32Application::GetHwnd();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;
	// Create Swap Chain
	IDXGISwapChain *swapChain;
	if (FAILED((factory->CreateSwapChain(
		m_commandQueue, 
		&swapChainDesc, 
		&swapChain
	)))) {
		throw std::exception("ERROR: Failed to create Swap Chain!");
	}
	//  ThrowIfFailed(swapChain.As(&m_swapChain)); // Not sure wtf this is supposed to do

	///  -------  Command Queue  -------

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

void D3D12Manager::initialize()
{
	this->loadPipeline();
	this->loadAssets();
	this->getHWND();
}

void D3D12Manager::render()
{
}

void D3D12Manager::update()
{
}

void D3D12Manager::destroy()
{
}

