#pragma once

#include <d3d12.h>
#include "ConstantBuffer.h"

class D3D12ConstantBuffer : public ConstantBuffer {
public:
	D3D12ConstantBuffer(std::string NAME, unsigned int location, ID3D12Device4 *device, const unsigned int frameCount, ID3D12DescriptorHeap* descriptorHeap);
	~D3D12ConstantBuffer();
	void setData(const void* data, size_t size, Material* m, unsigned int location);
	void bind(Material*);

private:
	ID3D12Device4 *m_device;
	ID3D12DescriptorHeap* m_descriptorHeap;
	unsigned int m_frameCount;
	std::string m_name;
	unsigned int m_location;
	ID3D12Resource1* m_constantBuffer;
	//GLuint handle;
	//GLuint index;
	//void* m_buff = nullptr;
	void* m_lastMat;
};