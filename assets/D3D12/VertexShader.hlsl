struct VSIn
{
	float3 pos		: POSITION;
	float3 color	: COLOR;
};

struct VSOut
{
	float4 pos		: SV_POSITION;
	float4 color	: COLOR;
};

cbuffer CB : register(b0)
{
	float R, G, B, A;
}

VSOut VS_main( VSIn input, uint index : SV_VertexID )
{
	VSOut output	= (VSOut)0;
	output.pos		= float4( input.pos, 1.0f );
	output.color	= float4(R, G, B, A);

	return output;
}