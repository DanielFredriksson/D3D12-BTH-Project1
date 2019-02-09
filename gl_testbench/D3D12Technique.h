#pragma once

#include "Technique.h"

#include "D3D12Material.h"
#include "D3D12RenderState.h"

#include <d3d12.h>

class D3D12Technique : public Technique {
private:
	ID3D12PipelineState* m_pipeLineState = nullptr;

public:
	D3D12Technique(Material* m, RenderState* r);
	~D3D12Technique();

	virtual void enable(Renderer* renderer);
};