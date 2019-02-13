struct VSOut {
	float4 position : SV_POSITION;
	float4 normal 	: NORMAL0;
	float2 texCoord : TEXCOORD0;
};

cbuffer DIFFUSE_TINT_NAME : register(b6) {
	float4 diffuseTint;
}

float4 PS_main(VSOut input) : SV_TARGET0 {
	float4 color = float4(1.0, 1.0, 1.0, 1.0);
	return color * float4(diffuseTint.rgb, 1.0);
}