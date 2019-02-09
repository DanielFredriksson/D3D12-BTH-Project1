#ifndef D3D12_TEXTURE_2D
#define D3D12_TEXTURE_2D

#include "Texture2D.h"
#include <DirectXMath.h>
#include "wrl.h"

//#include "..\include\Win32Application.h"
#include <dxgi.h>		// DirectX Graphics Infrastructure
#include <dxgi1_4.h>	// Enables IDXGIFactory4
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>


class D3D12Texture2D : public Texture2D
{
private:
	unsigned char* rgbTextureData;
	static const UINT TexturePixelSize = 4;    // The number of bytes used to represent a pixel in the texture.

	// Pipeline objects.
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_rtvDescriptorSize;

	// Resource Data
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	////////////////////////
	// ERROR CHECKING STUFF
	class HrException : public std::runtime_error
	{
	public:
		HrException(HRESULT hr) : std::runtime_error(HrToString(hr)), m_hr(hr) {}
		HRESULT Error() const { return m_hr; }
	private:
		const HRESULT m_hr;
		inline std::string HrToString(HRESULT hr)
		{
			char s_str[64] = {};
			sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
			return std::string(s_str);
		}
	};

	void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw HrException(hr);
		}
	}

	// END OF ERROR CHECKING STUFF
	//////////////////////////////

	UINT64 updateSubresources(
		ID3D12GraphicsCommandList* pCmdList,
		ID3D12Resource* pDestinationResource,
		ID3D12Resource* pIntermediate,
		UINT64 IntermediateOffset,
		UINT FirstSubresource,
		UINT NumSubresources,
		D3D12_SUBRESOURCE_DATA* pSrcData
	);

	UINT64 updateSubresourcesInternal(
		ID3D12GraphicsCommandList* pCmdList,
		ID3D12Resource* pDestinationResource,
		ID3D12Resource* pIntermediate,
		UINT FirstSubresource,
		UINT NumSubresources,
		UINT64 RequiredSize,
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
		UINT* pNumRows,
		UINT64* pRowSizesInBytes,
		D3D12_SUBRESOURCE_DATA* pSrcData
	);
	void WaitForPreviousFrame();
	
public:
	D3D12Texture2D();
	~D3D12Texture2D();

	int loadFromFile(std::string filename);
	void bind(unsigned int slot);
};

#endif
