#include "D3D12Material.h"

#include <d3d12.h> 
#include <d3dcompiler.h>
#include <dxgi.h>		// DirectX Graphics Infrastructure
#include <windows.h>
#include <streambuf>
#include <sstream>
#include <istream>
#include <fstream>
#include <vector>
#include <set>
#include <assert.h>
//////////////////////////

/*+-+-+-+-+-+-+-+-+-+-+-+
	PRIVATE FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

int D3D12Material::compileShader(ShaderType type, std::string& errString)
{	///////////////////////////////////////////
	//     PARAMETER #1 ('Source Data')     //
	/////////////////////////////////////////
	unsigned int shaderIndex = static_cast<unsigned int>(type);

	// open the file and read it to a string "shaderText"
	std::ifstream shaderFile(shaderFileNames[type]);
	std::string shaderText;
	if (shaderFile.is_open()) {
		shaderText = std::string((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());
		shaderFile.close();
	}
	else // Else we FAILED to read from the file
	{
		errString = "Cannot find file: " + shaderNames[shaderIndex];
		return -1;
	}

	// Combine all the individual 'define' strings into one string
	std::string shaderDefinesData = "";
	for (auto it = this->shaderDefines.at(type).begin(); it != this->shaderDefines.at(type).end(); it++)
		shaderDefinesData += *it;

	// Combine all of the 'define data' with the 'shader data'
	LPCVOID shaderSrcData = static_cast<LPCVOID>((shaderDefinesData + shaderText).c_str());



	//////////////////////////////////////////////////
	//     PARAMETER #2 ('Source Data LENGTH')     //
	////////////////////////////////////////////////
	SIZE_T shaderSrcDataLength = shaderText.length();



	//////////////////////////////////////////////////
	//     PARAMETER #2 ('Source Data LENGTH')     //
	////////////////////////////////////////////////
	std::string entryPointString;
	std::string shaderModelString;

	if (type == Material::ShaderType::VS)
	{
		entryPointString = "VS_main";
		shaderModelString = "vs_5_0";
	}
	else if (type == Material::ShaderType::PS)
	{
		entryPointString = "PS_main";
		shaderModelString = "ps_5_0";
	}
	else if (type == Material::ShaderType::GS)
	{
		entryPointString = "GS_main";
		shaderModelString = "gs_5_0";
	}
	else if (type == Material::ShaderType::CS)
	{
		entryPointString = "CS_main";
		shaderModelString = "cs_5_0";
	}

	LPCSTR entryPoint = entryPointString.c_str();
	LPCSTR shaderModel = shaderModelString.c_str();

	ID3DBlob* shaderDataBlob;
	D3DCompile(
		shaderSrcData,		// A pointer to uncompiled shader data; either ASCII HLSL code or a compiled effect.
		shaderSrcDataLength,// Length of 'shaderSrcData'
		nullptr,			// You can use this parameter for strings that specify error messages.
		nullptr,			// An array of NULL-terminated macro definitions
		nullptr,			// Optional. A pointer to an ID3DInclude for handling include files (ALREADY ADDED TO 'shaderSrcData')
		entryPoint,
		shaderModel,

	)

	this->shaderDefines

	// make final vector<string> with shader source + defines + GLSL version
	// in theory this uses move semantics (compiler does it automagically)
	std::vector<std::string> shaderLines = expandShaderText(shaderText, type);

	// OpenGL wants an array of GLchar* with null terminated strings 
	const GLchar** tempShaderLines = new const GLchar*[shaderLines.size()];
	int i = 0;
	for (std::string& text : shaderLines)
		tempShaderLines[i++] = text.c_str();

	GLuint newShader = glCreateShader(mapShaderEnum[shaderIdx]);
	glShaderSource(newShader, shaderLines.size(), tempShaderLines, nullptr);
	// ask GL to compile this
	glCompileShader(newShader);
	// print error or anything...
	INFO_OUT(newShader, Shader);
	std::string err2;
	COMPILE_LOG(newShader, Shader, err2);
	shaderObjects[shaderIdx] = newShader;
	return 0;
}

//std::vector<std::string> D3D12Material::expandShaderText(std::string& shaderText, Material::ShaderType type)
//{
//
//}




/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

void D3D12Material::setShader(const std::string& shaderFileName, ShaderType type)
{
	if (shaderFileNames.find(type) != shaderFileNames.end())
		removeShader(type);

	shaderFileNames[type] = shaderFileName;
}

void D3D12Material::removeShader(ShaderType type)
{
	unsigned int shaderType = this->shaderObjects[static_cast<unsigned int>(type)];

	this->shaderNames[shaderType] = "REMOVED";

	//if (this->shaderFileNames.find(type) == shaderFileNames.end())
	//{
	//	assert(shaderType == 0);
	//	return;
	//}

	//else if (shaderType != 0 && this->program != 0)
	//{

	//};
}

//void D3D12Material::setDiffuse(Color c)
//{
//
//}

//int D3D12Material::compileMaterial(std::string& errString)
//{
//
//}

//void D3D12Material::addConstantBuffer(std::string name, unsigned int location)
//{
//
//}

//void D3D12Material::updateConstantBuffer(const void* data, size_t size, unsigned int location)
//{
//
//}

//int D3D12Material::enable()
//{
//
//}

//void D3D12Material::disable()
//{
//
//}
