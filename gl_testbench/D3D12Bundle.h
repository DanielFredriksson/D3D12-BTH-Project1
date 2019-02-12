#pragma once

#include <d3d12.h>
#include <functional>
#include <vector>

#include "D3D12VertexBuffer.h"
#include "D3D12ConstantBuffer.h"

enum BUNDLE {
	BASIC, TECHNIQUE, COUNT
};

class D3D12Bundle {
private:
	// Global Objects
	ID3D12Device4*				gDevice5 = nullptr;
	ID3D12PipelineState*		gPipeLineState = nullptr;
	ID3D12RootSignature*		gRootSignature = nullptr;
	VertexBuffer*				gVertexBuffer = nullptr;

	void createD3D12BundleObjects();
	void populateBundle(
		D3D12_VIEWPORT*				 pViewPort,
		D3D12_RECT*					 pRect,
		D3D12_CPU_DESCRIPTOR_HANDLE* pcdh,
		ConstantBuffer*				 pCB,
		BUNDLE						 bundleType
	);

	void setGlobalObjects();


public:
	// Internal Objects
	ID3D12CommandAllocator*		bundleAllocator = nullptr;
	ID3D12CommandAllocator*		bundleCommandAllocator = nullptr;
	ID3D12GraphicsCommandList3*	bundle = nullptr;
	std::vector<std::function<void(D3D12Bundle*)>> commands;

	// Creates, Appends commands depending on bundleType, and closes the bundle
	D3D12Bundle();
	D3D12Bundle(ID3D12PipelineState* pPipeLineState);
	~D3D12Bundle();

	/*
	- Creates a D3D12 Bundle and populates it with pre-determined API-Calls.
	*/
	//void initialize(
	//	VertexBuffer*				pVertexBuffer,
	//	D3D12_VIEWPORT*				 pViewPort,
	//	D3D12_RECT*					 pRect,
	//	D3D12_CPU_DESCRIPTOR_HANDLE* pcdh,
	//	ConstantBuffer*				 pCB,
	//	BUNDLE						 bundleType
	//);
	/*
	- Releases internal D3D12 objects.
	- Clean is automatically called when the destructor is executed
	*/
	void clean();

	// UNIQUE BUNDLES ARE BASED ON THESE CALLS
	void appendBasicCommands(ConstantBuffer* pCB);
	void appendTechniqueCommands(ID3D12PipelineState* pPipeLineState);
	void addCommandsToBundle(std::function<void(D3D12Bundle*)> commandFunctions);

	void reset(ID3D12GraphicsCommandList3* mainCommandList);
	void appendBundleToCommandList(ID3D12GraphicsCommandList3* mainCommandList);
};