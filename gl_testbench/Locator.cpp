#include "Locator.h"

ID3D12RootSignature* Locator::gRootSignature = nullptr;
ID3D12Device4* Locator::gDevice = nullptr;
IDXGISwapChain3* Locator::gSwapChain = nullptr;
ID3D12GraphicsCommandList3* Locator::gCommandList = nullptr;
ID3D12CommandAllocator* Locator::gCommandAllocator = nullptr;