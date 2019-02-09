#define POSITION 0
#define NORMAL 1
#define TEXTCOORD 2
#define TRANSLATION 5
#define TRANSLATION_NAME TranslationBlock
#define DIFFUSE_TINT 6
#define DIFFUSE_TINT_NAME DiffuseColor
struct VSOut
{
	float4 pos		: SV_POSITION;
	float4 color	: COLOR;
};

float4 PS_main( VSOut input ) : SV_TARGET0
{
	return input.color;
}