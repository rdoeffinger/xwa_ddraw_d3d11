// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "config.h"
#include "common.h"

#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>

#include "joystick.h"

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

Config g_config;

Config::Config()
{
	this->AspectRatioPreserved = true;
	this->MultisamplingAntialiasingEnabled = false;
	this->AnisotropicFilteringEnabled = true;
	this->GenerateMipMaps = -1;
	this->WireframeFillMode = false;
	this->ScalingType = 0;
	this->Fullscreen = 0;
	this->Width = 0;
	this->Height = 0;
	this->JoystickEmul = -1;
	this->XInputTriggerAsThrottle = 0;
	this->InvertYAxis = false;
	this->MouseSensitivity = 0.5f;
	this->KbdSensitivity = 1.0f;
	this->XWAMode = true;
	int XWAModeInt = -1;
	int ProcessAffinity = -1;
	int AutoPatch = 2;

	this->Concourse3DScale = 0.6f;

	this->PresentSleepTime = -1;
	this->RefreshLimit = 1;

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
		}
	}
	char name[4096] = {};
	GetModuleFileNameA(NULL, name, sizeof(name));
	int len = strlen(name);

	bool isXWA = len >= 17 && _stricmp(name + len - 17, "xwingalliance.exe") == 0;
	isXWing = len >= 11 && _stricmp(name + len - 11, "xwing95.exe") == 0;
	isTIE = len >= 9 && _stricmp(name + len - 9, "tie95.exe") == 0;
	bool isXvT = len >= 11 && _stricmp(name + len - 11, "z_xvt__.exe") == 0 &&
		*(const unsigned long long *)0x523aec == 0x54534c2e31505352ull;
	bool isBoP = len >= 11 && _stricmp(name + len - 11, "z_xvt__.exe") == 0 &&
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
