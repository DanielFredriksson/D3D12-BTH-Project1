struct VSOut {
	float4 position : SV_POSITION;
	float4 normal 	: NORMAL0;
	float2 texCoord : TEXCOORD0;
};

Texture2D tex : register(t0);
SamplerState ss : register(s0);

cbuffer DIFFUSE_TINT_NAME : register(b6) {
	float4 diffuseTint;
}

float4 PS_main(VSOut input) : SV_TARGET0 {

	float4 color;
	
	#ifdef DIFFUSE_SLOT
		color = tex.Sample(ss, input.texCoord);
		return color;
	#else
		color = float4(1.0, 1.0, 1.0, 1.0);
		return color * float4(diffuseTint.rgb, 1.0);
	#endif
}