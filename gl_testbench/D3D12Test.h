#pragma once

#include <windows.h>
#include <d3d12.h>

#include <dxgi1_6.h> //Only used for initialization of the device and swap chain.
#include <d3dcompiler.h>

#include "Renderer.h"

LRESULT CALLBACK wndProc2(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); //Window Proc callback function

#pragma region globals
const unsigned int NUM_SWAP_BUFFERS = 2; //Number of buffers
#pragma endregion


template<class T> inline void SafeRelease2(T **ppInterface) {
	if (*ppInterface != NULL)
	{
		(*ppInterface)->Release();

		(*ppInterface) = NULL;
	}
}

class D3D12Test : public Renderer {
private:
	struct Vertex
	{
		float x, y, z; // Position
		float r, g, b; // Color
	};

	// Window- & HWND related data
	HWND wndHandle;

	unsigned int SCREEN_WIDTH, SCREEN_HEIGHT;


	ID3D12Device4*				gDevice5 = nullptr;
	ID3D12GraphicsCommandList3*	gCommandList4 = nullptr;

	ID3D12CommandQueue*			gCommandQueue = nullptr;
	ID3D12CommandAllocator*		gCommandAllocator = nullptr;
	IDXGISwapChain4*			gSwapChain4 = nullptr;

	ID3D12Fence1*				gFence = nullptr;
	HANDLE						gEventHandle = nullptr;
	UINT64						gFenceValue = 0;

	ID3D12DescriptorHeap*		gRenderTargetsHeap = nullptr;
	ID3D12Resource1*			gRenderTargets[NUM_SWAP_BUFFERS] = {};
	UINT						gRenderTargetDescriptorSize = 0;
	//UINT						gFrameIndex							= 0;

	D3D12_VIEWPORT				gViewport = {};
	D3D12_RECT					gScissorRect = {};

	ID3D12RootSignature*		gRootSignature = nullptr;
	ID3D12PipelineState*		gPipeLineState = nullptr;

	ID3D12Resource1*			gVertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW	gVertexBufferView = {};

#pragma region ConstantBufferGlobals
	struct ConstantBufferData
	{
		float colorChannel[4];
	};

	ID3D12DescriptorHeap*	gDescriptorHeap[NUM_SWAP_BUFFERS] = {};
	ID3D12Resource1*		gConstantBufferResource[NUM_SWAP_BUFFERS] = {};
	ConstantBufferData		gConstantBufferCPU = {};
#pragma endregion

#pragma region OwnVariables

	float m_clearColor[4] = { 0,0,0,0 };

	ConstantBuffer* m_testBuffer;

#pragma endregion

#pragma region MemberFunctions
	//----Member functions----
	HWND initWindow(unsigned int width = 800, unsigned int height = 600); // 1. Creates and returns a window

	//Helper function for syncronization of GPU/CPU
	void WaitForGpu();
	//Helper function for resource transitions - what?

	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	void CreateDirect3DDevice(HWND wndHandle);				//2. Create Device
	void CreateCommandInterfacesAndSwapChain(HWND wndHandle);	//3. Create CommandQueue and SwapChain
	void CreateFenceAndEventHandle();							//4. Create Fence and Event handle
	void CreateRenderTargets();									//5. Create render targets for backbuffer
	void CreateViewportAndScissorRect();						//6. Create viewport and rect
	void CreateShadersAndPiplelineState();						//7. Set up the pipeline state
	void CreateTriangleData();									//8. Create vertexdata
	void CreateRootSignature();
	void CreateConstantBufferResources();

	void	Update(int backBufferIndex);
	void	Render(int backBufferIndex);
	//------------------------
#pragma endregion
public:
	D3D12Test();
	~D3D12Test();

#pragma region InheritedFunctions
	///  ------  Inherited Functions  ------ 
	virtual Material* makeMaterial(const std::string& name);
	virtual Mesh* makeMesh();
	virtual VertexBuffer* makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage);
	virtual Texture2D* makeTexture2D();
	virtual Sampler2D* makeSampler2D();
	virtual RenderState* makeRenderState();
	virtual std::string getShaderPath();
	virtual std::string getShaderExtension();
	virtual ConstantBuffer* makeConstantBuffer(std::string NAME, unsigned int location);
	virtual Technique* makeTechnique(Material*, RenderState*);
	//
	virtual int initialize(unsigned int width = 800, unsigned int height = 600);
	virtual void setWinTitle(const char* title);
	virtual void present();
	virtual int shutdown();
	//
	virtual void setClearColor(float r, float g, float b, float a);
	virtual void clearBuffer(unsigned int);
	//
	virtual void setRenderState(RenderState *ps);
	//
	virtual void submit(Mesh* mesh);
	virtual void frame();
	/// ------------------------------------
#pragma endregion
};