#pragma once

#include <string>
#include <vector>

struct XwaText
{
	int positionX;
	int positionY;
	unsigned int color;
	int fontSize;
	char textChar;
};

extern std::vector<XwaText> g_xwa_text;

void RenderCharHook(short x, short y, unsigned char fw, unsigned char fh, char c, unsigned int color);

void ComputeMetricsHook();
