struct VSOut {
	float4 position : SV_POSITION;
	float4 normal 	: NORMAL0;
	float2 texCoord : TEXCOORD0;
};

#ifdef DIFFUSE_SLOT
//Texture2D tex : register(t0);
//SamplerState ss : register(s0);
#endif

cbuffer DIFFUSE_TINT_NAME : register(b6) {
	float4 diffuseTint;
}

float4 PS_main(VSOut input) : SV_TARGET0 {

#ifdef DIFFUSE_SLOT
    //float4 color = tex.Sample(ss, input.texCoord);
#else
 	//float4 color = float4(1.0, 1.0, 1.0, 1.0);
#endif

	float4 color = float4(1.0, 1.0, 1.0, 1.0);
	//float4 color = diffuseTint;

	return color * float4(diffuseTint.rgb, 1.0);
}