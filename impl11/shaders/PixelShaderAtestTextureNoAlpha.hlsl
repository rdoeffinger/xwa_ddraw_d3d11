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
	// The TIE engine does not expect 1-bit alpha to be interpolated,
	// so treat anything < 1 as transparent.
	// Do not check after multiply, as there seems to be random noise
	// causing alpha to not be exactly 1.0, and using an epsilon looks
	// a bit worse
	if (texelColor.a < 1 || input.color.a == 0) discard;
	texelColor *= input.color;
	return texelColor;
}
