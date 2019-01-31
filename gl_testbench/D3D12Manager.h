#pragma once

#include "Renderer.h"

/// D3D12
#include <d3d12.h>

#include <C:/Program Files (x86)/Windows Kits/10/Include/10.0.16299.0/um/d3d12.h>

#include <dxgi.h>		// DirectX Graphics Infrastructure
#include <dxgi1_4.h>	// Enables IDXGIFactory4

/// DEBUGGING
#include <exception>

/// GETHWND TESTING
/*
This function probably works by getting every windows hWnd and iterating through them
*/
//BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam);


/// WHAT IS THIS FOR?
template<class T> inline void SafeRelease(T **ppInterface);



class D3D12Manager : public Renderer {
private:
	static const UINT frameCount = 2;

	// Pipeline Objects
	D3D12_VIEWPORT m_viewPort;
	IDXGISwapChain3 *m_swapChain = nullptr;
	ID3D12Device *m_device = nullptr;

	ID3D12Resource *m_renderTargets[frameCount];

	ID3D12CommandAllocator *m_commandAllocator = nullptr;
	ID3D12CommandQueue *m_commandQueue;

	ID3D12RootSignature *m_rootSignature = nullptr;
	ID3D12DescriptorHeap *m_rtvHeap = nullptr;
	ID3D12PipelineState *m_pipelineState = nullptr;
	ID3D12GraphicsCommandList *m_commandList = nullptr;
	UINT m_rtvDescriptorSize;

	// App Resources
	ID3D12Resource *m_vertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	// Synchronization Objects
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ID3D12Fence *m_fence;
	UINT64 m_fenceValue;

	// Window- & HWND related data

	// Used by Private Functions
	void getHardwareAdapter(IDXGIFactory4 * pFactory, IDXGIAdapter1 ** ppAdapter);
	HWND *getHWND();

	// Used when clearing RTV
	float m_clearColor[4] = { 0,0,0,0 };

	// Used by Public functions
	void loadPipeline();
	void loadAssets();

	// Render-related functions
	void PopulateCommandList();

	void WaitForPreviousFrame();

public:
	D3D12Manager();
	~D3D12Manager();

	///  ------  Inherited Functions  ------ 
#pragma region
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
#pragma endregion
	/// ------------------------------------

};

template<class T>
inline void SafeRelease(T ** ppInterface)
{
	if (*ppInterface != NULL) {
		(*ppInterface)->Release();
		(*ppInterface) = NULL;
	}
}
