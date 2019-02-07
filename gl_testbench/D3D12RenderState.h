#pragma once

#include <d3d12.h> 

#include "RenderState.h"

class D3D12RenderState : public RenderState {
private:
	bool m_wireFrame;

public:
	D3D12RenderState();
	~D3D12RenderState();

	virtual void setWireFrame(bool state);

	virtual void set();

	void set(D3D12_GRAPHICS_PIPELINE_STATE_DESC *gpsd);
};