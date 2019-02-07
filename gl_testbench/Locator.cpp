#include "Locator.h"

ID3D12RootSignature* Locator::gRootSignature = nullptr;
ID3D12Device4* Locator::gDevice = nullptr;
ID3D12PipelineState* Locator::gPipelineState = nullptr;
IDXGISwapChain3* Locator::gSwapChain = nullptr;
ID3D12GraphicsCommandList3* Locator::gCommandList = nullptr;