#include "common.h"
#include "XwaDrawRadarHook.h"

std::vector<XwaRadar> g_xwa_radar;

int g_xwa_radar_selected_positionX = -1;
int g_xwa_radar_selected_positionY = -1;

void DrawRadarHook(XwaRadar* A4, short A8)
{
	for (int index = 0; index < A8; index++)
	{
		XwaRadar xwaRadar;

		xwaRadar.positionX = A4[index].positionX;
		xwaRadar.positionY = A4[index].positionY;
		xwaRadar.colorIndex = A4[index].colorIndex;

		g_xwa_radar.push_back(xwaRadar);
	}
}

void DrawRadarSelectedHook()
{
	short s_V0x068C8FC = *(short*)0x068C8FC;
	short s_V0x068C900 = *(short*)0x068C900;

	if (s_V0x068C8FC == -1 || s_V0x068C900 == -1)
	{
		return;
	}

	g_xwa_radar_selected_positionX = s_V0x068C8FC;
	g_xwa_radar_selected_positionY = s_V0x068C900;
}
