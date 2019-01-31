#include "OpenGL/OpenGLRenderer.h"
#include "Renderer.h"
#include "D3D12Manager.h"

Renderer* Renderer::makeRenderer(BACKEND option)
{
	if (option == BACKEND::GL45)
		return new OpenGLRenderer();
	else if (option == BACKEND::DX12) {
		return new D3D12Manager();
	}
}

