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

	// Combine all of the 'define data' with the 'shader data' saving as 'LPCVOID'
	LPCVOID shaderSrcData = static_cast<LPCVOID>((shaderDefinesData + shaderText).c_str());



	//////////////////////////////////////////////////
	//     PARAMETER #2 ('Source Data LENGTH')     //
	////////////////////////////////////////////////
	SIZE_T shaderSrcDataLength = shaderText.length();



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

	LPCSTR entryPoint = entryPointString.c_str();
	LPCSTR shaderModel = shaderModelString.c_str();



	///////////////////////////////////////////////
	//     PARAMETER #10, #11 ('Blob Data')     //
	/////////////////////////////////////////////
	ID3DBlob* shaderDataBlob;
	ID3DBlob* errorDataBlob;

	D3DCompile(
		shaderSrcData,		// A pointer to uncompiled shader data; either ASCII HLSL code or a compiled effect.
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
	);
	MessageBoxA(0, (char*)errorDataBlob->GetBufferPointer(), "", 0); // Error handling

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
	//		� Creation
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};
	//		� Specify pipeline stages
	gpsd.pRootSignature = Locator::getRootSignature();
	gpsd.InputLayout = inputLayoutDesc;
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(this->m_shaderDataBlob_VS->GetBufferPointer());
	gpsd.VS.BytecodeLength = this->m_shaderDataBlob_VS->GetBufferSize();
	gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(this->m_shaderDataBlob_PS->GetBufferPointer());
	gpsd.PS.BytecodeLength = this->m_shaderDataBlob_PS->GetBufferSize();
	//		� Specify render target and depthstencil usage
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;
	gpsd.SampleDesc.Count = 1;
	gpsd.SampleMask = UINT_MAX;
	//		� Specify rasterizer behaviour
	gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	//		� Specify blend descriptions
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
	};
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	ID3D12Device* tempDevice;
	Locator::getRootSignature()->GetDevice(IID_PPV_ARGS(&tempDevice));

	tempDevice->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&this->m_pipelineState));

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
