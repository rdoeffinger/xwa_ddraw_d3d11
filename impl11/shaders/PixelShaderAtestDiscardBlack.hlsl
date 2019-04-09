// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt


Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR0;
	float2 tex : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 texelColor = texture0.Sample(sampler0, input.tex);
	if (input.color.a == 0) discard;
	if (texelColor.r == 0 && texelColor.g == 0 && texelColor.b == 0) discard;
	texelColor *= input.color;
	return float4(texelColor.rgb, input.color.a);
}
