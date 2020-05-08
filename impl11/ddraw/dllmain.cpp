// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#define STRICT
#include <windows.h>

#include "XwaDrawTextHook.h"
#include "XwaDrawRadarHook.h"
#include "XwaDrawBracketHook.h"

bool IsXwaExe()
{
	char filename[4096];

	if (GetModuleFileName(nullptr, filename, sizeof(filename)) == 0)
	{
		return false;
	}

	int length = strlen(filename);

	if (length < 17)
	{
		return false;
	}

	return _stricmp(filename + length - 17, "xwingalliance.exe") == 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if (IsXwaExe())
		{
			// RenderCharHook
			*(unsigned char*)(0x00450A47 + 0x00) = 0xE8;
			*(int*)(0x00450A47 + 0x01) = (int)RenderCharHook - (0x00450A47 + 0x05);

			// ComputeMetricsHook
			*(unsigned char*)(0x00510385 + 0x00) = 0xE8;
			*(int*)(0x00510385 + 0x01) = (int)ComputeMetricsHook - (0x00510385 + 0x05);

			// DrawRadarHook
			*(int*)(0x00434977 + 0x06) = (int)DrawRadarHook;
			*(int*)(0x00434995 + 0x06) = (int)DrawRadarSelectedHook;

			// DrawBracketInFlightHook
			*(unsigned char*)(0x00503D46 + 0x00) = 0xE8;
			*(int*)(0x00503D46 + 0x01) = (int)DrawBracketInFlightHook - (0x00503D46 + 0x05);

			// DrawBracketInFlightHook CMD
			*(unsigned char*)(0x00478E44 + 0x00) = 0xE8;
			*(int*)(0x00478E44 + 0x01) = (int)DrawBracketInFlightHook - (0x00478E44 + 0x05);

			// DrawBracketMapHook
			*(unsigned char*)(0x00503CFE + 0x00) = 0xE8;
			*(int*)(0x00503CFE + 0x01) = (int)DrawBracketMapHook - (0x00503CFE + 0x05);
		}

		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
