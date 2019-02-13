#include "D3D12Sampler2D.h"
//////////////////////////

/*+-+-+-+-+-+-+-+-+-+-+-+
	PRIVATE FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

D3D12_STATIC_SAMPLER_DESC* D3D12Sampler2D::createSampler()
{
	//if (this->minFilter == FILTER::LINEAR && this->magFilter == FILTER::LINEAR) {
	//	this->samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	//}
	//else if (this->minFilter == FILTER::POINT_SAMPLER && this->magFilter == FILTER::LINEAR) {
	//	this->samplerDesc.Filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	//}
	//else if (this->minFilter == FILTER::LINEAR && this->magFilter == FILTER::POINT_SAMPLER) {
	//	this->samplerDesc.Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	//}
	//else if (this->minFilter == FILTER::POINT_SAMPLER && this->magFilter == FILTER::POINT_SAMPLER) {
	//	this->samplerDesc.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	//}

	//if (this->wrapS == WRAPPING::CLAMP) {
	//	this->samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	//}
	//else if (this->wrapS == WRAPPING::REPEAT) {
	//	this->samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	//}

	//if (this->wrapT == WRAPPING::CLAMP) {
	//	this->samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	//}
	//else if (this->wrapT == WRAPPING::REPEAT) {
	//	this->samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	//}

	//this->samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	//this->samplerDesc.MipLODBias = 0;
	//this->samplerDesc.MaxAnisotropy = 0;
	//this->samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	//this->samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	//this->samplerDesc.MinLOD = 0.0f;
	//this->samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	//this->samplerDesc.ShaderRegister = 0;
	//this->samplerDesc.RegisterSpace = 0;
	//this->samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	//return &this->samplerDesc;
	return nullptr;
}





/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

D3D12Sampler2D::D3D12Sampler2D()
{
	this->minFilter = FILTER::LINEAR;
	this->magFilter = FILTER::LINEAR;

	this->wrapS = WRAPPING::CLAMP;
	this->wrapT = WRAPPING::CLAMP;

	//this->createSampler();
}

D3D12Sampler2D::~D3D12Sampler2D()
{

}

void D3D12Sampler2D::setMagFilter(FILTER filter)
{
	this->magFilter = filter;
}

void D3D12Sampler2D::setMinFilter(FILTER filter)
{
	this->minFilter = filter;
}

void D3D12Sampler2D::setWrap(WRAPPING s, WRAPPING t)
{
	this->wrapS = s;
	this->wrapT = t;
}
