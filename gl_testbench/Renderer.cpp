#include "OpenGL/OpenGLRenderer.h"
#include "Renderer.h"
#include "D3D12Renderer.h"
#include "D3D12Test.h"

Renderer* Renderer::makeRenderer(BACKEND option)
{
	if (option == BACKEND::GL45)
		return new OpenGLRenderer();
	else if (option == BACKEND::DX12) {
		return new D3D12Renderer();
	}
	else if (option == BACKEND::DX12TEST) {
		return new D3D12Test();
	}
}

