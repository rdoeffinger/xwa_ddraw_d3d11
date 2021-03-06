// Copyright (c) 2014 J�r�my Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "config.h"
#include "common.h"

#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>

#include "joystick.h"
#include "XwaDrawTextHook.h"
#include "XwaDrawRadarHook.h"
#include "XwaDrawBracketHook.h"

// Wrapper to avoid undefined behaviour due to
// sign-extending char to int when passing to isspace.
// That would be undefined behaviour and crashes with table
// based implamentations.
// In addition it avoids issues due to ambiguity between
// multiple isspace overloads when used in a template context.
static int isspace_wrapper(char c)
{
    return std::isspace(static_cast<unsigned char>(c));
}

static void patchTimeGetTime(unsigned *addr)
{
	DWORD old, dummy;
	VirtualProtect(addr, 4, PAGE_READWRITE, &old);
	*addr = reinterpret_cast<unsigned>(emulGetTime);
	VirtualProtect(addr, 4, old, &dummy);
}

Config g_config;

Config::Config()
{
	this->AspectRatioPreserved = true;
	this->MultisamplingAntialiasingEnabled = false;
	this->AnisotropicFilteringEnabled = true;
	this->GenerateMipMaps = -1;
	this->VSyncEnabled = true;
	this->WireframeFillMode = false;
	this->ScalingType = 0;
	this->Fullscreen = 0;
	this->Width = 0;
	this->Height = 0;
	this->JoystickEmul = -1;
	this->XInputTriggerAsThrottle = 0;
	this->InvertYAxis = false;
	this->InvertThrottle = false;
	this->DisableDeadzone = true;
	this->MouseSensitivity = 0.5f;
	this->KbdSensitivity = 1.0f;
	this->XWAMode = true;
	int XWAModeInt = -1;
	int ProcessAffinity = -1;
	this->AutoPatch = 2;
	this->RuntimeAutoPatchDone = false;

	this->Concourse3DScale = 0.6f;

	this->PresentSleepTime = -1;
	this->RefreshLimit = 1;

	this->D3dHookExists = false;

	this->TextFontFamily = L"Verdana";
	this->TextWidthDelta = 0;

	if (std::ifstream("Hook_D3d.dll"))
	{
		this->D3dHookExists = true;
	}

	// Try to always load config from executable path, not CWD
	char execPath[MAX_PATH] = "";
	HMODULE module = GetModuleHandle(NULL);
	if (module)
	{
		GetModuleFileNameA(module, execPath, sizeof(execPath));
		char *end = strrchr(execPath, '\\');
		if (end)
		{
			end[1] = 0;
		}
		else
		{
			execPath[0] = 0;
		}
	}

	std::ifstream file(std::string(execPath) + "ddraw.cfg");

	if (file.is_open())
	{
		std::string line;

		while (std::getline(file, line))
		{
			if (!line.length())
			{
				continue;
			}

			if (line[0] == '#' || line[0] == ';' || (line[0] == '/' && line[1] == '/'))
			{
				continue;
			}

			int pos = line.find("=");

			std::string name = line.substr(0, pos);
			name.erase(remove_if(name.begin(), name.end(), isspace_wrapper), name.end());

			std::string value = line.substr(pos + 1);
			value.erase(remove_if(value.begin(), value.end(), isspace_wrapper), value.end());

			if (!name.length() || !value.length())
			{
				continue;
			}

			if (name == "PreserveAspectRatio")
			{
				this->AspectRatioPreserved = stoi(value) != 0;
			}
			else if (name == "EnableMultisamplingAntialiasing")
			{
				this->MultisamplingAntialiasingEnabled = stoi(value) != 0;
			}
			else if (name == "EnableAnisotropicFiltering")
			{
				this->AnisotropicFilteringEnabled = stoi(value) != 0;
			}
			else if (name == "GenerateMipMaps")
			{
				this->GenerateMipMaps = stoi(value);
			}
			else if (name == "EnableVSync")
			{
				this->VSyncEnabled = stoi(value) != 0;
			}
			else if (name == "FillWireframe")
			{
				this->WireframeFillMode = stoi(value) != 0;
			}
			else if (name == "ScalingType")
			{
				this->ScalingType = stoi(value);
			}
			else if (name == "Fullscreen")
			{
				this->Fullscreen = stoi(value);
			}
			else if (name == "Width")
			{
				this->Width = stoi(value);
			}
			else if (name == "Height")
			{
				this->Height = stoi(value);
			}
			else if (name == "JoystickEmul")
			{
				this->JoystickEmul = stoi(value);
			}
			else if (name == "InvertYAxis")
			{
				this->InvertYAxis = stoi(value) != 0;
			}
			else if (name == "InvertThrottle")
			{
				this->InvertThrottle = stoi(value) != 0;
			}
			else if (name == "DisableDeadzone")
			{
				this->DisableDeadzone = stoi(value) != 0;
			}
			else if (name == "MouseSensitivity")
			{
				this->MouseSensitivity = stof(value);
			}
			else if (name == "KbdSensitivity")
			{
				this->KbdSensitivity = stof(value);
			}
			else if (name == "XWAMode")
			{
				XWAModeInt = stoi(value);
			}
			else if (name == "AutoPatch")
			{
				AutoPatch = stoi(value);
			}
			else if (name == "Concourse3DScale")
			{
				this->Concourse3DScale = stof(value);
			}
			else if (name == "ProcessAffinity")
			{
				ProcessAffinity = stoi(value);
			}
			else if (name == "PresentSleepTime")
			{
				this->PresentSleepTime = stoi(value);
			}
			else if (name == "RefreshLimit")
			{
				this->RefreshLimit = stoi(value);
			}
			else if (name == "XInputTriggerAsThrottle")
			{
				this->XInputTriggerAsThrottle = stoi(value);
			}
			else if (name == "TextFontFamily")
			{
				this->TextFontFamily = string_towstring(value);
			}
			else if (name == "TextWidthDelta")
			{
				this->TextWidthDelta = stoi(value);
			}
		}
	}
	char name[4096] = {};
	GetModuleFileNameA(NULL, name, sizeof(name));
	int len = strlen(name);

	isXWA = len >= 17 && _stricmp(name + len - 17, "xwingalliance.exe") == 0;
	isXWing = len >= 11 && _stricmp(name + len - 11, "xwing95.exe") == 0;
	isTIE = len >= 9 && _stricmp(name + len - 9, "tie95.exe") == 0;
	isXvT = len >= 11 && _stricmp(name + len - 11, "z_xvt__.exe") == 0 &&
		*(const unsigned long long *)0x523aec == 0x54534c2e31505352ull;
	isBoP = len >= 11 && _stricmp(name + len - 11, "z_xvt__.exe") == 0 &&
		*(const unsigned long long *)0x5210bc == 0x54534c2e31505352ull;

	if (XWAModeInt == -1)
	{
		this->XWAMode = isXWA;
	}
	else
	{
		this->XWAMode = XWAModeInt != 0;
	}
	if (this->XWAMode && this->GenerateMipMaps == -1)
		this->GenerateMipMaps = 0;

	// softwarecursor, prefer second version modifying data section as that works with Steam.
	if (0 && AutoPatch >= 1 && isXWing && *(const unsigned *)0x4ac038 == 0x1c74c085u) {
		*(unsigned *)0x4ac038 = 0x9090c085u;
	}
	if (AutoPatch >= 1 && isXWing && *(const unsigned *)0x4ded80 == 0x74666f73u) {
		*(unsigned *)0x4ded80 = 0;
	}
	// softwarecursor, prefer second version modifying data section as that works with Steam.
	if (0 && AutoPatch >= 1 && isTIE && *(const unsigned *)0x499d7e == 0x1c74c085u) {
		*(unsigned *)0x4ac038 = 0x9090c085u;
	}
	if (AutoPatch >= 1 && isTIE && *(const unsigned *)0x4f3e54 == 0x74666f73u) {
		*(unsigned *)0x4f3e54 = 0;
	}

	if (AutoPatch >= 2 && RefreshLimit == 1)
	{
		if (isBoP) patchTimeGetTime((unsigned *)0xbb96b8);
		if (isXvT) patchTimeGetTime((unsigned *)0x86070c);
		if (isXWing) patchTimeGetTime((unsigned *)0x4c31ec);
		if (isTIE) patchTimeGetTime((unsigned *)0x4dc264);
	}

	if (this->JoystickEmul != 0 && isXWing)
	{
		// TODO: How to check if this is a supported binary?
		DWORD old, dummy;
		VirtualProtect((void *)0x4c31f0, 0x4c3208 - 0x4c31f0, PAGE_READWRITE, &old);
		*(unsigned *)0x4c31f0 = reinterpret_cast<unsigned>(emulJoyGetDevCaps);
		*(unsigned *)0x4c3200 = reinterpret_cast<unsigned>(emulJoyGetPosEx);
		*(unsigned *)0x4c3204 = reinterpret_cast<unsigned>(emulJoyGetNumDevs);
		VirtualProtect((void *)0x4c31f0, 0x4c3208 - 0x4c31f0, old, &dummy);
	}

	if (this->JoystickEmul != 0 && isTIE)
	{
		// TODO: How to check if this is a supported binary?
		DWORD old, dummy;
		VirtualProtect((void *)0x4dc258, 0x4dc264 - 0x4dc258, PAGE_READWRITE, &old);
		*(unsigned *)0x4dc258 = reinterpret_cast<unsigned>(emulJoyGetNumDevs);
		*(unsigned *)0x4dc25c = reinterpret_cast<unsigned>(emulJoyGetDevCaps);
		*(unsigned *)0x4dc260 = reinterpret_cast<unsigned>(emulJoyGetPosEx);
		VirtualProtect((void *)0x4dc258, 0x4dc264 - 0x4dc258, old, &dummy);
	}

	if (this->JoystickEmul != 0 && isXvT)
	{
		// TODO: How to check if this is a supported binary?
		DWORD old, dummy;
		VirtualProtect((void *)0x860704, 0x860714 - 0x860704, PAGE_READWRITE, &old);
		*(unsigned *)0x860704 = reinterpret_cast<unsigned>(emulJoyGetDevCaps);
		*(unsigned *)0x860708 = reinterpret_cast<unsigned>(emulJoyGetNumDevs);
		*(unsigned *)0x860710 = reinterpret_cast<unsigned>(emulJoyGetPosEx);
		VirtualProtect((void *)0x860704, 0x860714 - 0x860704, old, &dummy);
	}

	if (this->JoystickEmul != 0 && isBoP)
	{
		// TODO: How to check if this is a supported binary?
		DWORD old, dummy;
		VirtualProtect((void *)0xbb969c, 0xbb96b0 - 0xbb969c, PAGE_READWRITE, &old);
		*(unsigned *)0xbb969c = reinterpret_cast<unsigned>(emulJoyGetNumDevs);
		*(unsigned *)0xbb96a8 = reinterpret_cast<unsigned>(emulJoyGetPosEx);
		*(unsigned *)0xbb96ac = reinterpret_cast<unsigned>(emulJoyGetDevCaps);
		VirtualProtect((void *)0xbb969c, 0xbb96b0 - 0xbb969c, old, &dummy);
	}

	if (this->JoystickEmul != 0 && isXWA)
	{
		// TODO: How to check if this is a supported binary?
		DWORD old, dummy;
		VirtualProtect((void *)0x5a92a4, 0x5a92b8 - 0x5a92a4, PAGE_READWRITE, &old);
		*(unsigned *)0x5a92a4 = reinterpret_cast<unsigned>(emulJoyGetPosEx);
		*(unsigned *)0x5a92a8 = reinterpret_cast<unsigned>(emulJoyGetDevCaps);
		*(unsigned *)0x5a92b4 = reinterpret_cast<unsigned>(emulJoyGetNumDevs);
		VirtualProtect((void *)0x5a92a4, 0x5a92b8 - 0x5a92a4, old, &dummy);
	}

	if (ProcessAffinity != 0) {
		DWORD_PTR CurProcessAffinity, SystemAffinity;
		HANDLE mod = GetCurrentProcess();
		if (GetProcessAffinityMask(mod, &CurProcessAffinity, &SystemAffinity)) {
			if (ProcessAffinity > 0) {
				ProcessAffinity &= SystemAffinity;
			} else {
				ProcessAffinity = (SystemAffinity & 2) ? 2 : 1;
			}
		}
		SetProcessAffinityMask(mod, ProcessAffinity);
	}

	DisableProcessWindowsGhosting();
}

static bool patchCall(int address, void *dst) {
	if (*(unsigned char*)(address) != 0xE8) return false;
	DWORD old, dummy;
	VirtualProtect((void *)address, 5, PAGE_READWRITE, &old);
	*(int*)(address + 0x01) = (int)dst - (address + 0x05);
	VirtualProtect((void *)address, 5, old, &dummy);
	return true;
}

void Config::runAutopatch()
{
	if (RuntimeAutoPatchDone) return;
	RuntimeAutoPatchDone = true;
	static const int all_deadzone_addrs[][5] = {
		// last address duplicated for all but XWA, to avoid a special case
		{0x40b4c0, 0x4acafa, 0x4acb3f, 0x4acb83, 0x4acb83}, // X-Wing
		{0x41ce80, 0x49a91a, 0x49a95f, 0x49a9a3, 0x49a9a3}, // TIE
		{0x4115f0, 0x4aae78, 0x4aaec5, 0x4aaf12, 0x4aaf12}, // BoP
		{0x405590, 0x4a5a43, 0x4a5a8f, 0x4a5adb, 0x4a5adb}, // XvT
		{0x4344a0, 0x50ba69, 0x50baae, 0x50bb00, 0x50bb4b}, // XWA
		{0, 0, 0, 0, 0} // Invalid
	};
	const int (&deadzone_addrs)[5] = all_deadzone_addrs[isXWing ? 0 : isTIE ? 1 : isBoP ? 2 : isXvT ? 3 : isXWA ? 4 : 5];
	if (DisableDeadzone && deadzone_addrs[0]) {
		if (*(const unsigned char*)deadzone_addrs[0] == 0x51 || *(const unsigned char*)deadzone_addrs[0] == 0x66) {
			DWORD old, dummy;
			VirtualProtect((void*)deadzone_addrs[0], 1, PAGE_READWRITE, &old);
			*(unsigned char*)deadzone_addrs[0] = 0xc3;
			VirtualProtect((void*)deadzone_addrs[0], 1, old, &dummy);
		}
		// More patches needed (replacing jle by NOP, jg for XvT because cmp arguments are swapped):
		bool match = true;
		for (int i = 1; i < 5; ++i) {
			match &= *(const unsigned char*)deadzone_addrs[i] == (isXvT ? 0x7d : 0x7e);
		}
		if (match)
		{
			DWORD old, dummy;
			int len = deadzone_addrs[4] + 2 - deadzone_addrs[1];
			VirtualProtect((void*)deadzone_addrs[1], len, PAGE_READWRITE, &old);
			for (int i = 1; i < 5; ++i) {
				*(unsigned short *)deadzone_addrs[i] = 0x9090;
			}
			VirtualProtect((void*)deadzone_addrs[1], len, old, &dummy);
		}
	}
	// ISD laser fix, not tested (esp. with Steam version)
	if (AutoPatch >= 2 && isTIE &&
		*(const unsigned long long *)0x4df898 == 0x3735353433323130ull &&
		*(const unsigned long long *)0x4f0830 == 0x0400210045003443ull &&
		*(const unsigned long long *)0x4f0844 == 0x0103000235430000ull &&
		*(const unsigned long long *)0x4f084c == 0x000003840c1c0300ull) {
		*(unsigned long long *)0x4df898 = 0x3736363433323130ull;
		*(unsigned long long *)0x4f0830 = 0x0400210345003443ull;
		*(unsigned long long *)0x4f0844 = 0x2103450034430000ull;
		*(unsigned long long *)0x4f084c = 0x00007d00fa000400ull;
	}
	// Disable mipmapping
	if (AutoPatch >= 2 && isXWing && GenerateMipMaps &&
		*(const unsigned long long *)0x47f676 == 0xc0d9004c32a03dd8ull) {
		DWORD old, dummy;
		VirtualProtect((void *)0x47f676, 8, PAGE_READWRITE, &old);
		*(unsigned long long *)0x47f676 = 0xc0d9004c329c3dd8ull;
		VirtualProtect((void *)0x47f676, 8, old, &dummy);
	}
	if (AutoPatch >= 2 && isTIE && GenerateMipMaps &&
		*(const unsigned long long *)0x42835f == 0xc0d9004dc3283dd8ull) {
		DWORD old, dummy;
		VirtualProtect((void *)0x42835f, 8, PAGE_READWRITE, &old);
		*(unsigned long long *)0x42835f = 0xc0d9004dc3243dd8ull;
		VirtualProtect((void *)0x42835f, 8, old, &dummy);
	}
	// Change LOD distance factor constant from 1 to 1/32 for more detailed models
	if (AutoPatch >= 2 && isXWing &&
		*(const unsigned *)0x4cef64 == 0x3f800000u) {
		DWORD old, dummy;
		VirtualProtect((void *)0x4cef64, 4, PAGE_READWRITE, &old);
		*(unsigned *)0x4cef64 = 0x3d000000u;
		VirtualProtect((void *)0x4cef64, 4, old, &dummy);
	}
	if (AutoPatch >= 2 && isTIE &&
		*(const unsigned *)0x4f2a8c == 0x3f800000u) {
		DWORD old, dummy;
		VirtualProtect((void *)0x4f2a8c, 4, PAGE_READWRITE, &old);
		*(unsigned *)0x4f2a8c = 0x3d000000u;
		VirtualProtect((void *)0x4f2a8c, 4, old, &dummy);
	}
#ifndef __MINGW32__
	if (AutoPatch >= 2 && isXWA && g_config.XWAMode) {
		// RenderCharHook
		patchCall(0x00450A47, RenderCharHook);

		// ComputeMetricsHook
		patchCall(0x00510385, ComputeMetricsHook);

		// DrawRadarHook
		DWORD old, dummy;
		VirtualProtect((void *)(0x00434977 + 0x6), 0x00434995 - 0x00434977 + 4, PAGE_READWRITE, &old);
		*(int*)(0x00434977 + 0x06) = (int)DrawRadarHook;
		*(int*)(0x00434995 + 0x06) = (int)DrawRadarSelectedHook;
		VirtualProtect((void *)(0x00434977 + 0x6), 0x00434995 - 0x00434977 + 4, old, &dummy);

		// DrawBracketInFlightHook
		patchCall(0x00503D46, DrawBracketInFlightHook);

		// DrawBracketInFlightHook CMD
		patchCall(0x00478E44, DrawBracketInFlightHook);

		// DrawBracketMapHook
		patchCall(0x00503CFE, DrawBracketMapHook);
	}
#endif
	FlushInstructionCache(GetCurrentProcess(), NULL, 0);
}
