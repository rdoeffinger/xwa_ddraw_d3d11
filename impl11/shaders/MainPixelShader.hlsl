// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt


Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);
float2 texSize : register(b0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float4 texelColor = texture0.Sample(sampler0, input.tex);

	int2 intcoord = floor(input.tex * texSize);

	float4 pointSampleColor = texture0.Load(int3(intcoord, 0));
	if ((abs(pointSampleColor.r - (4.0/31.0)) < 0.001 || abs(pointSampleColor.r - (32.0 / 255.0)) < 0.001) &&
		pointSampleColor.g == 0 && pointSampleColor.b == 0)
	{
		discard;
	}

	return texelColor;
}
