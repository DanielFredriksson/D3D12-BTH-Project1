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

	#ifdef NORMAL
	 	output.normal = input.normal;
	#endif

	#ifdef TEXTCOORD
		
		output.texCoord = input.texCoord;
		if (input.texCoord.r == -0.05f &&
			input.texCoord.g == -0.05f) {
			output.texCoord = float2(1.0, 1.0);
		}
		/*if (input.texCoord.r == 0.5f &&
			input.texCoord.g == 0.99f) {
			output.texCoord = float2(1.0, 1.0);
		}
		else if (
			input.texCoord.r == 0.49f &&
			input.texCoord.g == 0.1f
		) {
			output.texCoord = float2(0.8, 1.0);
		}
		else if (
			input.texCoord.r == 0.51f &&
			input.texCoord.g == 0.1f
			) {
			output.texCoord = float2(0.7, 1.0);
		}*/
	 #endif


	return output;
}