#include "D3D12VertexBuffer.h"

#include "Locator.h"

D3D12VertexBuffer::D3D12VertexBuffer(size_t size) {
	m_device = Locator::getDevice();
	m_commandList4 = Locator::getCommandList();

	m_bufferSize = size;
	//m_usage = usage;

	//Note: using upload heaps to transfer static data like vert buffers is not 
	//recommended. Every time the GPU needs it, the upload heap will be marshalled 
	//over. Please read up on Default Heap usage. An upload heap is used here for 
	//code simplicity and because there are very few vertices to actually transfer.
	D3D12_HEAP_PROPERTIES hp = {};
	hp.Type = D3D12_HEAP_TYPE_UPLOAD;
	hp.CreationNodeMask = 1;
	hp.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC rd = {};
	rd.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	rd.Width = size;
	rd.Height = 1;
	rd.DepthOrArraySize = 1;
	rd.MipLevels = 1;
	rd.SampleDesc.Count = 1;
	rd.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Creates both a resource and an implicit heap, such that the heap is big enough
	//to contain the entire resource and the resource is mapped to the heap. 
	m_device->CreateCommittedResource(
		&hp,
		D3D12_HEAP_FLAG_NONE,
		&rd,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBufferResource));

	m_vertexBufferResource->SetName(L"vb heap");
}

D3D12VertexBuffer::~D3D12VertexBuffer() {
	if (m_vertexBufferResource != nullptr) {
		m_vertexBufferResource->Release();
	}
}

void D3D12VertexBuffer::setData(const void* data, size_t size, size_t offset) {
	//Copy the triangle data to the vertex buffer.
	void* dataBegin = nullptr;
	D3D12_RANGE range = { 0, 0 }; //We do not intend to read this resource on the CPU.
	m_vertexBufferResource->Map(0, &range, &dataBegin);
	memcpy(dataBegin, data, size);
	m_vertexBufferResource->Unmap(0, nullptr);

	//Initialize vertex buffer view, used in the render call.
	m_vertexBufferView.BufferLocation = m_vertexBufferResource->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = (UINT)offset;
	m_vertexBufferView.SizeInBytes = (UINT)size;
}

void D3D12VertexBuffer::bind(size_t offset, size_t size, unsigned int location) {
	//If this doesn't work it probably is because we have to convert size to number of elements instead of data size
	m_commandList4->IASetVertexBuffers((UINT)offset, (UINT)size, &m_vertexBufferView);
}

void D3D12VertexBuffer::bindBundle(ID3D12GraphicsCommandList* commandList, size_t offset, size_t size, unsigned int location)
{
	commandList->IASetVertexBuffers((UINT)offset, (UINT)size, &m_vertexBufferView);
}

void D3D12VertexBuffer::unbind() {

}

size_t D3D12VertexBuffer::getSize() {
	return m_bufferSize;
}
