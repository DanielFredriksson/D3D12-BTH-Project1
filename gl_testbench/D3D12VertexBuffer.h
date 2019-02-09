#pragma once

#include <d3d12.h>

#include "VertexBuffer.h"

class D3D12VertexBuffer : public VertexBuffer {
private:
	ID3D12Device4 *m_device;
	ID3D12GraphicsCommandList3*	m_commandList4;

	size_t m_bufferSize;
	//DATA_USAGE m_usage;

	ID3D12Resource1*			m_vertexBufferResource = nullptr;
	D3D12_VERTEX_BUFFER_VIEW	m_vertexBufferView = {};

public:
	D3D12VertexBuffer(size_t size);
	virtual ~D3D12VertexBuffer();
	
	virtual void setData(const void* data, size_t size, size_t offset);
	virtual void bind(size_t offset, size_t size, unsigned int location);
	void bindBundle(ID3D12GraphicsCommandList* commandList, size_t offset, size_t size, unsigned int location);
	virtual void unbind();
	virtual size_t getSize();
};