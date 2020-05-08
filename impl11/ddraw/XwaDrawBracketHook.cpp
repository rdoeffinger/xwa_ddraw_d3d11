#include "common.h"
#include "XwaDrawBracketHook.h"

std::vector<XwaBracket> g_xwa_bracket;

void DrawBracketInFlightHook(int A4, int A8, int AC, int A10, unsigned char A14, int A18)
{
	XwaBracket bracket;

	bracket.positionX = A4;
	bracket.positionY = A8;
	bracket.width = AC;
	bracket.height = A10;
	bracket.colorIndex = A14;
	bracket.depth = A18;

	if (bracket.depth == 1)
	{
		bracket.positionX += *(short*)0x07D5244;
		bracket.positionY += *(short*)0x07CA354;
	}

	g_xwa_bracket.push_back(bracket);
}

void DrawBracketMapHook(int A4, int A8, int AC, int A10, unsigned char A14)
{
	XwaBracket bracket;

	bracket.positionX = A4;
	bracket.positionY = A8;
	bracket.width = AC;
	bracket.height = A10;
	bracket.colorIndex = A14;
	bracket.depth = 0;

	g_xwa_bracket.push_back(bracket);
}
