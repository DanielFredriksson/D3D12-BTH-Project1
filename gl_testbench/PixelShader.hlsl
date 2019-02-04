struct VSout
{
	float4 position : SV_POSITION;
	float4 color	: COLOR0;
};

float4 PS_main(VSout input) : SV_TARGET0 
{
	return input.color;
}