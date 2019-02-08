#pragma once

#include <d3d12.h>
#include "D3D12VertexBuffer.h"

class D3D12Bundle {
private:
	
	// Global Objects
	ID3D12Device4*				gDevice5 = nullptr;
	ID3D12PipelineState*		gPipeLineState = nullptr;
	ID3D12RootSignature*		gRootSignature = nullptr;
	VertexBuffer*				gVertexBuffer = nullptr;

	void createD3D12BundleObjects();
	void populateBundle();

public:
	// Internal Objects
	ID3D12CommandAllocator*		bundleAllocator = nullptr;
	ID3D12CommandAllocator*		bundleCommandAllocator = nullptr;
	ID3D12GraphicsCommandList3*	bundle = nullptr;
	//----- Should be in private, only for testing

	D3D12Bundle();
	~D3D12Bundle();

	/*
	- Creates a D3D12 Bundle and populates it with pre-determined API-Calls.
	*/
	void initialize(VertexBuffer* pVertexBuffer);
	/*
	- Releases internal D3D12 objects.
	- Clean is automatically called when the destructor is executed
	*/
	void clean();

	void reset(ID3D12GraphicsCommandList3* mainCommandList);
	void appendBundleToCommandList(ID3D12GraphicsCommandList3* mainCommandList);
};