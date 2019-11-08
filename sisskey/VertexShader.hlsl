
struct ObjectConstants
{
	float4x4 WVP;
};

ConstantBuffer<ObjectConstants> cb : register(b0);

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

	float4 pos = float4(input.pos, .0f, 1.f);

	output.pos = mul(pos, cb.WVP);
	output.col = input.col;

	return output;
}