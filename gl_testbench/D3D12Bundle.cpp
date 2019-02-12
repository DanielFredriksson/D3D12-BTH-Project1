#include "D3D12Bundle.h"

#include "Locator.h"
#include <d3d12.h>
#include <functional>

void D3D12Bundle::createD3D12BundleObjects()
{
	///  ------  Create Bundle Components  ------ 
	// Create Bundle Allocator
	ThrowIfFailed(gDevice5->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_BUNDLE,
		IID_PPV_ARGS(&bundleAllocator)
	));
	bundleAllocator->SetName(L"bundleAllocator");

	// Create Bundle Command Allocator
	ThrowIfFailed(gDevice5->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&bundleCommandAllocator)
	));
	bundleCommandAllocator->SetName(L"bundleCommandAllocator");

	// Create the Bundle
	ThrowIfFailed(gDevice5->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_BUNDLE,
		bundleAllocator,				// Where is the stack?
		gPipeLineState,
		IID_PPV_ARGS(&bundle)			// Where is the list?
	));
	bundle->SetName(L"bundle");

	// Closing the bundle is omitted since commands are recorded directly afterwards.
}


D3D12Bundle::D3D12Bundle()
{
}

D3D12Bundle::D3D12Bundle(ID3D12PipelineState* pPipeLineState)
{
	// Creation of D3D12 Objects (Bundle is open)
	this->setGlobalObjects();
	this->createD3D12BundleObjects();

	// Currently only using bundles for techniques
	
	this->appendTechniqueCommands(pPipeLineState);

	this->bundle->Close();
}

D3D12Bundle::~D3D12Bundle()
{
	this->clean();
}

//void D3D12Bundle::initialize(
//	VertexBuffer*				 pVertexBuffer,
//	D3D12_VIEWPORT*				 pViewPort,
//	D3D12_RECT*					 pRect,
//	D3D12_CPU_DESCRIPTOR_HANDLE* pcdh,
//	ConstantBuffer*				 pCB,
//	BUNDLE						 bundleType
//)
//{
//	// Set Global Objects
//	this->gDevice5 = Locator::getDevice();
//	this->gPipeLineState = Locator::getPipelineState();
//	this->gRootSignature = Locator::getRootSignature();
//	this->gVertexBuffer = pVertexBuffer;
//
//	// Create and populate bundles
//	this->createD3D12BundleObjects();
//	this->populateBundle(
//		pViewPort,
//		pRect,
//		pcdh,
//		pCB,
//		bundleType
//	);
//}

void D3D12Bundle::clean()
{
	// Release Intenal Objects
	SafeRelease(&bundleAllocator);
	SafeRelease(&bundleCommandAllocator);
	SafeRelease(&bundle);
}

void D3D12Bundle::populateBundle(
	D3D12_VIEWPORT*				 pViewPort,
	D3D12_RECT*					 pRect,
	D3D12_CPU_DESCRIPTOR_HANDLE* cdh,
	ConstantBuffer*				 pCB,
	BUNDLE						 bundleType
)
{
	/// BUNDLED COMMANDS
	//if (BUNDLE::BASIC) {
	//	this->appendBasicCommands();
	//}
	//else if (BUNDLE::TEST){
	//	this->appendTestCommands();
	//}
	
	bundle->Close();
}

void D3D12Bundle::setGlobalObjects()
{
	this->gDevice5 = Locator::getDevice();
	this->gPipeLineState = Locator::getPipelineState();
	this->gRootSignature = Locator::getRootSignature();
}

void D3D12Bundle::appendBasicCommands(ConstantBuffer* pCB)
{
	bundle->SetGraphicsRootSignature(gRootSignature);
	bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	static_cast<D3D12ConstantBuffer*>(pCB)->bindBundle(bundle, nullptr);
	static_cast<D3D12VertexBuffer*>(gVertexBuffer)->bindBundle(bundle, 0, 1, 0);
	bundle->DrawInstanced(6, 2, 0, 0);
}

void D3D12Bundle::appendTechniqueCommands(ID3D12PipelineState* pPipeLineState)
{
	//this->reset(Locator::getCommandList());
	this->bundle->SetPipelineState(pPipeLineState);
}


void D3D12Bundle::addCommandsToBundle(std::function<void(D3D12Bundle*)> commandFunctions)
{
	this->commands.push_back(commandFunctions);
}

void D3D12Bundle::reset(ID3D12GraphicsCommandList3 * mainCommandList)
{
	// Reset the stack
	ThrowIfFailed(bundleCommandAllocator->Reset());
	// Reset the main command list
	ThrowIfFailed(mainCommandList->Reset(bundleCommandAllocator, gPipeLineState));
}

void D3D12Bundle::appendBundleToCommandList(ID3D12GraphicsCommandList3 * mainCommandList)
{
	//this->reset(Locator::getCommandList());
	mainCommandList->ExecuteBundle(bundle);
}
