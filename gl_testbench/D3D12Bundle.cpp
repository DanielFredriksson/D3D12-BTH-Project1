#include "D3D12Bundle.h"

#include "Locator.h"
#include <d3d12.h>


void D3D12Bundle::createD3D12BundleObjects()
{
	///  ------  Create Bundle Components  ------ 
	// Create Bundle Allocator
	ThrowIfFailed(gDevice5->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_BUNDLE,
		IID_PPV_ARGS(&bundleAllocator)
	));

	// Create Bundle Command Allocator
	ThrowIfFailed(gDevice5->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&bundleCommandAllocator)
	));

	// Create the Bundle
	ThrowIfFailed(gDevice5->CreateCommandList(
		0,						
		D3D12_COMMAND_LIST_TYPE_BUNDLE,
		bundleAllocator,				// Where is the stack?
		nullptr,
		IID_PPV_ARGS(&bundle)			// Where is the list?
	));

	// Closing the bundle is omitted since commands are recorded directly afterwards.
}

void D3D12Bundle::populateBundle()
{
	/// BUNDLED COMMANDS
	bundle->SetPipelineState(gPipeLineState);	// Added since debug complains, but why not for normal?

	bundle->SetGraphicsRootSignature(gRootSignature); // Needed?
	bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gVertexBuffer->bind(0, 1, 0); 
	bundle->DrawInstanced(6, 2, 0, 0);

	//bundle->Close();
}

D3D12Bundle::D3D12Bundle()
{
}

D3D12Bundle::~D3D12Bundle()
{
	this->clean();
}

void D3D12Bundle::initialize(VertexBuffer* pVertexBuffer)
{
	// Set Global Objects
	this->gDevice5 = Locator::getDevice();
	this->gPipeLineState = Locator::getPipelineState();
	this->gRootSignature = Locator::getRootSignature();
	this->gVertexBuffer = pVertexBuffer;

	// Create and populate bundles
	this->createD3D12BundleObjects();
	this->populateBundle();
}

void D3D12Bundle::clean()
{
	// Release Intenal Objects
	SafeRelease(&bundleAllocator);
	SafeRelease(&bundleCommandAllocator);
	SafeRelease(&bundle);
}

void D3D12Bundle::reset(ID3D12GraphicsCommandList3 * mainCommandList)
{
	// Reset the stack
	ThrowIfFailed(bundleAllocator->Reset());
	// Reset the main command list
	ThrowIfFailed(mainCommandList->Reset(bundleAllocator, gPipeLineState));
}

void D3D12Bundle::appendBundleToCommandList(ID3D12GraphicsCommandList3 * mainCommandList)
{
	mainCommandList->ExecuteBundle(bundle);
}
