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
#include <iostream>

#include "Locator.h"
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

	// Extra step, just in case
	std::string allDataToBeConverted = (shaderDefinesData + shaderText);



	//////////////////////////////////////////////////
	//     PARAMETER #2 ('Source Data LENGTH')     //
	////////////////////////////////////////////////
	SIZE_T shaderSrcDataLength = static_cast<SIZE_T>(allDataToBeConverted.length());



	///////////////////////////////////////////////////////////////
	//     PARAMETER #6, #7 ('Entry Point', 'Shader Model')     //
	/////////////////////////////////////////////////////////////
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

	LPCSTR entryPoint = static_cast<LPCSTR>(entryPointString.c_str());
	LPCSTR shaderModel = static_cast<LPCSTR>(shaderModelString.c_str());



	///////////////////////////////////////////////
	//     PARAMETER #10, #11 ('Blob Data')     //
	/////////////////////////////////////////////
	ID3DBlob* shaderDataBlob;
	ID3DBlob* errorDataBlob;

	if (FAILED(D3DCompile(
		allDataToBeConverted.data(), // A pointer to uncompiled shader data; either ASCII HLSL code or a compiled effect.
		shaderSrcDataLength,// Length of 'shaderSrcData'
		nullptr,			// You can use this parameter for strings that specify error messages.
		nullptr,			// An array of NULL-terminated macro definitions
		nullptr,			// Optional. A pointer to an ID3DInclude for handling include files (ALREADY ADDED TO 'shaderSrcData')
		entryPoint,			// The name of the shader entry point function where shader execution begins.
		shaderModel,		// A string that specifies the shader target or set of shader features to compile against.
		0,					// Flags defined by D3D compile constants.
		0,					// Flags defined by D3D compile effect constants.
		&shaderDataBlob,	// A pointer to a variable that receives a pointer to the ID3DBlob interface that you can use to access the compiled code.
		&errorDataBlob		// A pointer to a variable that receives a pointer to the ID3DBlob interface that you can use to access compiler error messages.
	)))
	{
		MessageBoxA(0, (char*)errorDataBlob->GetBufferPointer(), "", 0); // Error handling
	}

	// THE SHADER HAS NOW BEEN SUCCESSFULLY CREATED ! ! !

	if (type == Material::ShaderType::VS)
		this->m_shaderDataBlob_VS = shaderDataBlob;

	else if (type == Material::ShaderType::PS)
		this->m_shaderDataBlob_PS = shaderDataBlob;

	else if (type == Material::ShaderType::GS)
		std::cout << "ERROR!";

	else if (type == Material::ShaderType::CS)
		std::cout << "ERROR!";

	return 0;
}

std::vector<std::string> D3D12Material::expandShaderText(std::string& shaderText, Material::ShaderType type)
{
	std::vector<std::string> result{ "\n\n #version 450\n\0" };
	for (auto define : shaderDefines[type])
		result.push_back(define);
	result.push_back(shaderText);
	return result;
}




/*+-+-+-+-+-+-+-+-+-+-+-+
	 PUBLIC FUNCTIONS
+-+-+-+-+-+-+-+-+-+-+-+*/

void D3D12Material::setShader(const std::string& shaderFileName, Material::ShaderType type)
{
	if (shaderFileNames.find(type) != shaderFileNames.end())
		removeShader(type);

	shaderFileNames[type] = shaderFileName;
}

void D3D12Material::removeShader(Material::ShaderType type)
{
	unsigned int shaderType = this->shaderObjects[static_cast<unsigned int>(type)];

	this->shaderNames[shaderType] = "REMOVED";

	if (type == Material::ShaderType::VS && this->m_shaderDataBlob_VS != nullptr)
		this->m_shaderDataBlob_VS->Release();

	else if (type == Material::ShaderType::PS && this->m_shaderDataBlob_PS != nullptr)
		this->m_shaderDataBlob_PS->Release();

	else if (type == Material::ShaderType::GS)
		std::cout << "ERROR!";

	else if (type == Material::ShaderType::CS)
		std::cout << "ERROR!";
}

void D3D12Material::setDiffuse(Color c)
{

}

int D3D12Material::compileMaterial(std::string& errString)
{
	// remove all shaders.
	removeShader(ShaderType::VS);
	removeShader(ShaderType::PS);

	// compile shaders
	std::string err;
	if (compileShader(ShaderType::VS, err) < 0) {
		errString = err;
		exit(-1);
	};
	if (compileShader(ShaderType::PS, err) < 0) {
		errString = err;
		exit(-1);
	};

	// try to link the program
	// link shader program (connect vs and ps)

	// Input Layout
	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
	inputLayoutDesc.pInputElementDescs = inputElementDesc;
	inputLayoutDesc.NumElements = ARRAYSIZE(inputElementDesc);

	// Pipeline State:
	//		� Specify pipeline stages
	this->gpsd.pRootSignature = Locator::getRootSignature();
	this->gpsd.InputLayout = inputLayoutDesc;
	this->gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	this->gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(this->m_shaderDataBlob_VS->GetBufferPointer());
	this->gpsd.VS.BytecodeLength = this->m_shaderDataBlob_VS->GetBufferSize();
	this->gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(this->m_shaderDataBlob_PS->GetBufferPointer());
	this->gpsd.PS.BytecodeLength = this->m_shaderDataBlob_PS->GetBufferSize();
	//		� Specify render target and depthstencil usage
	this->gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	this->gpsd.NumRenderTargets = 1;
	this->gpsd.SampleDesc.Count = 1;
	this->gpsd.SampleMask = UINT_MAX;
	//		� Specify rasterizer behaviour
	this->gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	this->gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//		� Specify blend descriptions
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
	};
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		this->gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	return 0;
}

void D3D12Material::addConstantBuffer(std::string name, unsigned int location)
{
	this->constantBuffers[location] = new D3D12ConstantBuffer(name, location);
}

void D3D12Material::updateConstantBuffer(const void* data, size_t size, unsigned int location)
{
	this->constantBuffers[location]->setData(data, size, this, location);
}

int D3D12Material::enable()
{
	ID3D12PipelineState* tempPointer = Locator::getPipelineState();
	Locator::getDevice()->CreateGraphicsPipelineState(&this->gpsd, IID_PPV_ARGS(&tempPointer));

	return 0;
}

void D3D12Material::disable()
{

}
