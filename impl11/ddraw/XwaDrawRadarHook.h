#pragma once

#include <vector>

#pragma pack(push, 1)

struct XwaRadar
{
	short positionX;
	short positionY;
	unsigned char colorIndex;
	char unk05[1];
};

static_assert(sizeof(XwaRadar) == 6, "size of S0x04D36F0 must be 6");

#pragma pack(pop)

extern std::vector<XwaRadar> g_xwa_radar;

extern int g_xwa_radar_selected_positionX;
extern int g_xwa_radar_selected_positionY;

void DrawRadarHook(XwaRadar* A4, short A8);

void DrawRadarSelectedHook();
