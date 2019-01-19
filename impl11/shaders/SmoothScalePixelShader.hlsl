// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt


Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};

float3 calc_sample_offsets(float coord)
{
	float4 tmp = float4(-0.5, 0.1666, 0.3333, -0.3333) * coord + float4(1, 0, -0.5, 0.5);
	tmp = tmp * coord + float4(0, 0, -0.5, 0.5);
	tmp = tmp * coord + float4(-0.6666, 0, 0.8333, 0.1666);
	tmp.xy /= tmp.zw;
	tmp.xy += float2(1 + coord, 1 - coord);
	return tmp.xyz;
}

float4 main(PixelShaderInput input) : SV_TARGET
{
	float2 texSize;
	texture0.GetDimensions(texSize.x, texSize.y);
	float2 coord = frac(input.tex * texSize + float2(0.5, 0.5));
	float3 offsetx = calc_sample_offsets(coord.x);
    offsetx.xy /= float2(-texSize.x, texSize.x);
	float3 offsety = calc_sample_offsets(coord.y);
    offsety.xy /= float2(-texSize.y, texSize.y);
	float4 a = texture0.Sample(sampler0, input.tex + float2(offsetx.x, offsety.x));
	float4 b = texture0.Sample(sampler0, input.tex + float2(offsetx.x, offsety.y));
	float4 c = texture0.Sample(sampler0, input.tex + float2(offsetx.y, offsety.x));
	float4 d = texture0.Sample(sampler0, input.tex + float2(offsetx.y, offsety.y));
	float4 texelColor = lerp(lerp(d, c, offsety.z), lerp(b, a, offsety.z), offsetx.z);

	return texelColor;
}
