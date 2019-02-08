#ifndef D3D12_TEXTURE_2D
#define D3D12_TEXTURE_2D

#include "Texture2D.h"

class D3D12Texture2D : public Texture2D
{
private:

	
public:
	D3D12Texture2D();
	~D3D12Texture2D();

	int loadFromFile(std::string filename);
	void bind(unsigned int slot);
};

#endif
