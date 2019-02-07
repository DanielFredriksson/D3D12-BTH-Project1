#include "D3D12RenderState.h"

D3D12RenderState::D3D12RenderState() {
	m_wireFrame = false;
}

D3D12RenderState::~D3D12RenderState() {

}

void D3D12RenderState::setWireFrame(bool state) {
	m_wireFrame = state;
}

void D3D12RenderState::set() {
	//Use this render state
}

void D3D12RenderState::set(D3D12_GRAPHICS_PIPELINE_STATE_DESC *gpsd) {
	//Set m_wireFrame to graphics pipeline state desc

	//		• Specify rasterizer behaviour
	gpsd->RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
}