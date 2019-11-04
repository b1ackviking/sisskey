
struct VSinput
{
	float2 pos : POSITION;
	float3 col : COLOR;
};

struct VSoutput
{
	float4 pos : SV_POSITION;
	float3 col : COLOR;
};

VSoutput main(VSinput input)
{
	VSoutput output = (VSoutput)0;

	output.pos = float4(input.pos, .0f, 1.f);
	output.col = input.col;

	return output;
}