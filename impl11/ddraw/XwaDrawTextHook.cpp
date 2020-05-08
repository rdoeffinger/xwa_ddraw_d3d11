#include "common.h"
#include "XwaDrawTextHook.h"

std::vector<XwaText> g_xwa_text;

void RenderCharHook(short x, short y, unsigned char fw, unsigned char fh, char c, unsigned int color)
{
	XwaText xwaText;

	xwaText.positionX = x;
	xwaText.positionY = y;
	xwaText.color = color;
	xwaText.fontSize = fh;

	char t[2];
	t[0] = c;
	t[1] = 0;
	xwaText.text = t;

	g_xwa_text.push_back(xwaText);
}

void ComputeMetricsHook()
{
	ComPtr<IDWriteFactory> dwriteFactory;
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(dwriteFactory), (IUnknown**)&dwriteFactory);

	int fontSizes[] = { 12, 16, 10 };
	unsigned char* fontWidths[] = { (unsigned char*)0x007D4C80, (unsigned char*)0x007D4D80, (unsigned char*)0x007D4E80 };

	for (int index = 0; index < 3; index++)
	{
		ComPtr<IDWriteTextFormat> textFormat;
		dwriteFactory->CreateTextFormat(
			g_config.TextFontFamily.c_str(),
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			(float)fontSizes[index],
			L"en-US",
			&textFormat);

		for (int i = 0; i < 256;i++)
		{
			char t[2];
			t[0] = (char)i;
			t[1] = 0;

			std::wstring wtext = string_towstring(t);

			ComPtr<IDWriteTextLayout> layout;
			dwriteFactory->CreateTextLayout(wtext.c_str(), 1, textFormat, 100, 100, &layout);

			DWRITE_TEXT_METRICS m{};
			layout->GetMetrics(&m);

			fontWidths[index][i] = (unsigned char)m.widthIncludingTrailingWhitespace + (unsigned char)g_config.TextWidthDelta;
		}
	}
}
