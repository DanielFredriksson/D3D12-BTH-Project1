#ifndef D3D12_MATERIAL_H
#define D3D12_MATERIAL_H

#include "Material.h"
#include <stdio.h>
#include <vector>
#include <string>

#include <d3d12.h> 
#include <d3dcompiler.h>
#include <dxgi.h>		// DirectX Graphics Infrastructure
#include "D3D12ConstantBuffer.h"

class D3D12Material : public Material
{
private:
	// map from ShaderType to GL_VERTEX_SHADER, should be static.
	unsigned int mapShaderEnum[4];
	std::string shaderNames[4];
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd;

	ID3DBlob* m_shaderDataBlob_PS = nullptr;
	ID3DBlob* m_shaderDataBlob_VS = nullptr;

	// opengl shader object
	unsigned int shaderObjects[4] = { 0,0,0,0 };

	// TODO: change to PIPELINE
	// opengl program object
	std::string name;
	unsigned int program;

	int compileShader(ShaderType type, std::string& errString);
	std::vector<std::string> expandShaderText(std::string& shaderText, ShaderType type);
	std::map<unsigned int, D3D12ConstantBuffer*> constantBuffers;

	unsigned int m_constantBufferIndex = -1;

public:
	D3D12Material(std::string name) : Material() { this->name = name; }
	virtual ~D3D12Material() { this->m_shaderDataBlob_VS->Release(); this->m_shaderDataBlob_PS->Release(); };

	// set shader name, DOES NOT COMPILE
	virtual void setShader(const std::string& shaderFileName, ShaderType type);

	// removes any resource linked to shader type
	virtual void removeShader(ShaderType type);

	virtual void setDiffuse(Color c);

	/*
	 * Compile and link all shaders
	 * Returns 0  if compilation/linking succeeded.
	 * Returns -1 if compilation/linking fails.
	 * Error is returned in errString
	 * A Vertex and a Fragment shader MUST be defined.
	 * If compileMaterial is called again, it should RE-COMPILE the shader
	 * In principle, it should only be necessary to re-compile if the defines set
	 * has changed.
	*/
	virtual int compileMaterial(std::string& errString);

	// this constant buffer will be bound every time we bind the material
	virtual void addConstantBuffer(std::string name, unsigned int location);

	// location identifies the constant buffer in a unique way
	virtual void updateConstantBuffer(const void* data, size_t size, unsigned int location);

	// activate the material for use.
	virtual int enable();

	//Allows us to send in the graphics 
	void enable(D3D12_GRAPHICS_PIPELINE_STATE_DESC *gpsd);

	// disable material
	virtual void disable();
};

#endif