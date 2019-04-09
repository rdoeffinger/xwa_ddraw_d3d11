// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#pragma once

class Config
{
public:
	Config();

	// delayed autopatch, run when we are sure binary is decrypted (for Steam version)
	void runAutopatch();

	bool AspectRatioPreserved;
	bool MultisamplingAntialiasingEnabled;
	bool AnisotropicFilteringEnabled;
	int GenerateMipMaps;
	bool WireframeFillMode;
	int ScalingType;

	int Fullscreen;
	int Width;
	int Height;
	int JoystickEmul;
	int XInputTriggerAsThrottle;
	bool InvertYAxis;
	float MouseSensitivity;
	float KbdSensitivity;
	bool XWAMode;
	bool isTIE;
	bool isXWing;
	bool isXvT;

	float Concourse3DScale;

	int PresentSleepTime;
	int RefreshLimit;

	int AutoPatch;
	bool RuntimeAutoPatchDone;
};

extern Config g_config;
