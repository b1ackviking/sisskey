struct VSoutput
{
	float4 pos : SV_POSITION;
	float3 col : COLOR;
};

float4 main(VSoutput input) : SV_TARGET
{
	return float4(input.col, 1.f);
}