#ifndef D3D12_SAMPLER_2D
#define D3D12_SAMPLER_2D

#include "Sampler2D.h"

class D3D12Sampler2D : public Sampler2D
{
private:


public:
	int magFilter, minFilter;
	int wrapS, wrapT;

	D3D12Sampler2D();
	virtual ~D3D12Sampler2D();

	void setMagFilter(FILTER filter);
	void setMinFilter(FILTER filter);
	void setWrap(WRAPPING s, WRAPPING t);
};

#endif
