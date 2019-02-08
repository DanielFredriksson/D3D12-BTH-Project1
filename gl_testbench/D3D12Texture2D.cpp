#include "D3D12Texture2D.h"

/*+-+-+-+-+-+-+-+-+-+-+-+
	PRIVATE FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/





/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

D3D12Texture2D::D3D12Texture2D()
{

}

D3D12Texture2D::~D3D12Texture2D()
{

}

int D3D12Texture2D::loadFromFile(std::string fileName)
{
	return 0;
}

void D3D12Texture2D::bind(unsigned int slot)
{

}


/*
	GUIDE TO LOADING A TEXTURE, DX12

	1. Load the data from an image
		a. Create a WIC factory
		b. Create a WIC Bitmap Decoder
		c. Grab a 'frame' from the decoder
		d. Get image information (such as width, height)
		e. Get the compatible DXGI Format
		f. If a compatible DXGI format was not found, we must convert the image to a WIC pixel format that IS compatible with a DXGI Format.
		g. Finally copy the pixels from the WIC Frame to a BYTE array

	2. Create an upload heap, default heap, and resource to store the bitmap data

	3. Create a shader resource view that describes and points to the bitmap image data.
*/
