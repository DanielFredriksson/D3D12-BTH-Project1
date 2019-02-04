#include "D3D12Material.h"

#include <d3d12.h> 
#include <d3dcompiler.h>
#include <dxgi.h>		// DirectX Graphics Infrastructure
//////////////////////////

/*+-+-+-+-+-+-+-+-+-+-+-+
	PRIVATE FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

int D3D12Material::compileShader(ShaderType type, std::string& errString)
{
	//// index in the the array "shaderObject[]";
	//GLuint shaderIdx = (GLuint)type;

	//// open the file and read it to a string "shaderText"
	//std::ifstream shaderFile(shaderFileNames[type]);
	//std::string shaderText;
	//if (shaderFile.is_open()) {
	//	shaderText = std::string((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
	//	shaderFile.close();
	//}
	//else {
	//	errString = "Cannot find file: " + shaderNames[shaderIdx];
	//	return -1;
	//}

	//// make final vector<string> with shader source + defines + GLSL version
	//// in theory this uses move semantics (compiler does it automagically)
	//std::vector<std::string> shaderLines = expandShaderText(shaderText, type);

	//// debug
	//for (auto ex : shaderLines)
	//	DBOUTW(ex.c_str());

	//// OpenGL wants an array of GLchar* with null terminated strings 
	//const GLchar** tempShaderLines = new const GLchar*[shaderLines.size()];
	//int i = 0;
	//for (std::string& text : shaderLines)
	//	tempShaderLines[i++] = text.c_str();

	//GLuint newShader = glCreateShader(mapShaderEnum[shaderIdx]);
	//glShaderSource(newShader, shaderLines.size(), tempShaderLines, nullptr);
	//// ask GL to compile this
	//glCompileShader(newShader);
	//// print error or anything...
	//INFO_OUT(newShader, Shader);
	//std::string err2;
	//COMPILE_LOG(newShader, Shader, err2);
	//shaderObjects[shaderIdx] = newShader;
	//return 0;

	return 0;
}

std::vector<std::string> expandShaderText(std::string& shaderText, Material::ShaderType type)
{
	std::vector<std::string> temp;
	return temp;
}




/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

void D3D12Material::setShader(const std::string& shaderFileName, ShaderType type)
{
	if (shaderFileNames.find(type) != shaderFileNames.end())
	{
		removeShader(type);
	}
	shaderFileNames[type] = shaderFileName;
}

void D3D12Material::removeShader(ShaderType type)
{
	//int shaderType = shaderObject

	//GLuint shader = shaderObjects[(GLuint)type];
	//// check if name exists (if it doesn't there should not be a shader here.
	//if (shaderFileNames.find(type) == shaderFileNames.end())
	//{
	//	assert(shader == 0);
	//	return;
	//}
	//else if (shader != 0 && program != 0) {
	//	glDetachShader(program, shader);
	//	glDeleteShader(shader);
	//};
}

void D3D12Material::setDiffuse(Color c)
{

}

int D3D12Material::compileMaterial(std::string& errString)
{
	// Remove all shaders
	this->removeShader(ShaderType::PS);
	this->removeShader(ShaderType::VS);
	this->removeShader(ShaderType::GS);
	this->removeShader(ShaderType::CS);

	// Compile all shaders
	std::string error;
	if (0 < compileShader(ShaderType::VS, error)) {
		errString = error;
		return -1;
	}
	if (0 < compileShader(ShaderType::PS, error)) {
		errString = error;
		return -1;
	}

	return 0;
}

void D3D12Material::addConstantBuffer(std::string name, unsigned int location)
{

}

void D3D12Material::updateConstantBuffer(const void* data, size_t size, unsigned int location)
{

}

int D3D12Material::enable()
{
	return 0;
}

void D3D12Material::disable()
{

}
