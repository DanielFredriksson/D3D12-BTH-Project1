#include "Locator.h"

SDL_Window* Locator::gWindow = nullptr;
ID3D12RootSignature* Locator::gRootSignature = nullptr;
ID3D12Device4* Locator::gDevice = nullptr;
ID3D12PipelineState* Locator::gPipelineState = nullptr;