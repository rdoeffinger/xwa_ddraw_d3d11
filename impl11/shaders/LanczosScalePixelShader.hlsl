// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt


Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};

static const float pi = 3.14159265358979323846;
static const int range = 2;
static const float A = 0.5;
static const float B = 0.85;

float L(float d)
{
	if (d*d == 0) return A*B;
	return sin(pi * B * d) * sin(pi * A * d) * (1.0 / (d * d));
}

float4 main(PixelShaderInput input) : SV_TARGET
{
	float2 texSize;
	texture0.GetDimensions(texSize.x, texSize.y);

	float2 coord = input.tex * texSize - float2(0.5, 0.5);
	float2 fpart = frac(coord);
	int2 base = floor(coord);
	float coeff[2*range];
	float factor_x = 0;
	for (int x = -range + 1; x <= range; x++)
	{
		coeff[x + range - 1] = L(x - fpart.x);
		factor_x += coeff[x + range - 1];
	}
	float4 sum = 0;
	float factor_y = 0;
	for (int y = -range + 1; y <= range; y++)
	{
		float4 row = 0;
		for (int x = -range + 1; x <= range; x++)
		{
			float4 v = texture0.Load(int3(base + int2(x, y), 0));
			row += v*coeff[x + range - 1];
		}
		float tmp = L(y - fpart.y);
		sum += row * tmp;
		factor_y += tmp;
	}
	sum *= 1.0 / (factor_x * factor_y);

	return sum;
}
