#include "D3D12ConstantBuffer.h"
#include "Locator.h"

D3D12ConstantBuffer::D3D12ConstantBuffer(std::string NAME, unsigned int location)
{
	m_device = Locator::getDevice();
	m_swapChain = Locator::getSwapChain();
	m_commandList4 = Locator::getCommandList();

	m_name = NAME;
	m_location = location;

	m_lastMat = nullptr;

	//m_hasBeenInitialized = false;

	//if this is the first time we are running setData - do this.
	//if (!m_hasBeenInitialized) {
	for (unsigned int i = 0; i < m_frameCount; i++)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
		heapDescriptorDesc.NumDescriptors = 1;
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		m_device->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&m_descriptorHeap[i]));
	}

	// Heap Properties
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1;	// Used when multi-gpu
	heapProperties.VisibleNodeMask = 1;		// Used when multi-gpu
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	// Resource Description
	UINT cbSizeAligned = 1024 * 64; //???
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = cbSizeAligned;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Create a resource heap, descriptor heap, and pointer to cbv for each frame
	for (unsigned int i = 0; i < m_frameCount; i++) {
		// Committed Resource - creates implicit heap with the heapProperties
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

	//}

}

D3D12ConstantBuffer::~D3D12ConstantBuffer()
{
	for (int i = 0; i < 2; i++) {
		if (m_descriptorHeap[i] != NULL) {
			m_descriptorHeap[i]->Release();
		}
		if (m_constantBufferResource[i] != NULL) {
			m_constantBufferResource[i]->Release();
		}
	}
}

void D3D12ConstantBuffer::setData(const void * data, size_t size, Material * m, unsigned int location)
{
	m_location = location;

	int backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	//Update GPU memory
	void* mappedMem = nullptr;
	D3D12_RANGE readRange = { 0, 0 }; //We do not intend to read this resource on the CPU.
	if (SUCCEEDED(m_constantBufferResource[backBufferIndex]->Map(0, &readRange, &mappedMem)))
	{
		memcpy(mappedMem, data, size);

		D3D12_RANGE writeRange = { 0, size };
		m_constantBufferResource[backBufferIndex]->Unmap(0, &writeRange);
	}

	//m_hasBeenInitialized = true;
}

void D3D12ConstantBuffer::bind(Material *)
{
	int backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

	//Set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_descriptorHeap[backBufferIndex] };
	m_commandList4->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

	//Set root descriptor table to index 0 in previously set root signature
	m_commandList4->SetGraphicsRootDescriptorTable(
		0,
		m_descriptorHeap[backBufferIndex]->GetGPUDescriptorHandleForHeapStart()
	);
}
