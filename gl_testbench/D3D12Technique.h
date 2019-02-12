#pragma once

#include "Technique.h"

#include "D3D12Material.h"
#include "D3D12RenderState.h"

#include <d3d12.h>
#include "D3D12Bundle.h"

class D3D12Technique : public Technique {
private:
	ID3D12PipelineState* m_pipeLineState = nullptr;
	D3D12Bundle* m_Bundle = nullptr;

	void initBundle();

public:
	D3D12Technique(Material* m, RenderState* r);
	~D3D12Technique();

	virtual void enable(Renderer* renderer);
};