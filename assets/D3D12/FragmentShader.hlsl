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
//	float u = clamp(input.texCoord, 0, 1);
//	float v = clamp(input.texCoord, 0, 1);
//	color = float4(u, v, 0.0, 0.0);
//	color = tex.Sample(ss, float2(u, v));
	color = tex.Sample(ss, input.texCoord);
//	color = float4(input.texCoord.r, input.texCoord.g, 0.0f, 1.0f);
#else
	color = float4(diffuseTint.rgb, 1.0);
#endif

	return color;
}