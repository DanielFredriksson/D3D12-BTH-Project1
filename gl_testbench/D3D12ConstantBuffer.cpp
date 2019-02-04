#include "D3D12ConstantBuffer.h"

D3D12ConstantBuffer::D3D12ConstantBuffer(std::string NAME, unsigned int location, ID3D12Device4 *device, const unsigned int frameCount, ID3D12DescriptorHeap* descriptorHeap)
{
	m_device = device;

	m_frameCount = frameCount;

	m_descriptorHeap = descriptorHeap;

	m_name = NAME;
	m_location = location;

	m_lastMat = nullptr;
}

D3D12ConstantBuffer::~D3D12ConstantBuffer()
{

}

void D3D12ConstantBuffer::setData(const void * data, size_t size, Material * m, unsigned int location)
{

	//if descriptor heaps has not been created - do this.
	for (int i = 0; i < m_frameCount; i++)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
		heapDescriptorDesc.NumDescriptors = 1;
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		m_device->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&m_descriptorHeap));
	}



	//Update GPU memory
	void* mappedMem = nullptr;
	D3D12_RANGE readRange = { 0, 0 }; //We do not intend to read this resource on the CPU.
	if (SUCCEEDED(m_constantBuffer->Map(0, &readRange, &mappedMem)))
	{
		memcpy(mappedMem, data, size);

		D3D12_RANGE writeRange = { 0, size };
		m_constantBuffer->Unmap(0, &writeRange);
	}
}

void D3D12ConstantBuffer::bind(Material *)
{

}
