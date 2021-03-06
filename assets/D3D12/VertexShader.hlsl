struct VSIn {
	float4 position : POSITION0;
	float4 normal 	: NORMAL0;
	float2 texCoord : TEXCOORD0;
};

struct VSOut {
	float4 position : SV_POSITION;
	float4 normal 	: NORMAL0;
	float2 texCoord : TEXCOORD0;
};

cbuffer TRANSLATION_NAME : register(b5) {
	float4 translate;
}

VSOut VS_main(VSIn input) {
	VSOut output = (VSOut)0;
	output.position = input.position + translate;
	//output.position = input.position;

#ifdef NORMAL
	output.normal = input.normal;
#endif

#ifdef TEXTCOORD
	output.texCoord = input.texCoord;
#endif

	return output;
}