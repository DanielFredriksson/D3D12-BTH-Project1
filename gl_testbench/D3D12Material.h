#ifndef D3D12_MATERIAL_H
#define D3D12_MATERIAL_H

#include "Material.h"
#include <string>

class D3D12Material : public Material
{
private:
	std::string name;

public:
	D3D12Material(std::string name) : Material() { this->name = name; }
	virtual ~D3D12Material() {};

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

	// disable material
	virtual void disable();
};

#endif
