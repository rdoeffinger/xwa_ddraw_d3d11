// Copyright (c) 2016-2018 Reimar Döffinger
// Licensed under the MIT license. See LICENSE.txt

#include "config.h"
#include "common.h"

#include <mmsystem.h>
#include <xinput.h>

#include "joystick.h"

#pragma comment(lib, "winmm")
#pragma comment(lib, "XInput9_1_0")

#undef min
#undef max
#include <algorithm>

static int needsJoyEmul()
{
	JOYCAPS caps = {};
	if (joyGetDevCaps(0, &caps, sizeof(caps)) != JOYERR_NOERROR ||
	    !(caps.wCaps & JOYCAPS_HASZ) || caps.wNumAxes <= 2 ||
	    // Steam controller
	    (caps.wMid == 0x45e && caps.wPid == 0x28e))
	{
		// Probably the joystick is just an emulation from a gamepad.
		// Rather try to use it as gamepad directly then.
		XINPUT_STATE state;
		if (XInputGetState(0, &state) == ERROR_SUCCESS) return 2;
	}
	UINT cnt = joyGetNumDevs();
	for (unsigned i = 0; i < cnt; ++i)
	{
		JOYINFOEX jie;
		memset(&jie, 0, sizeof(jie));
		jie.dwSize = sizeof(jie);
		jie.dwFlags = JOY_RETURNALL;
		UINT res = joyGetPosEx(0, &jie);
		if (res == JOYERR_NOERROR)
			return 0;
	}
	return 1;
}

UINT WINAPI emulJoyGetNumDevs(void)
{
	if (g_config.JoystickEmul < 0) {
		g_config.JoystickEmul = needsJoyEmul();
	}
	if (!g_config.JoystickEmul) {
		return joyGetNumDevs();
	}
	return 1;
}

static UINT joyYmax;

UINT WINAPI emulJoyGetDevCaps(UINT_PTR joy, struct tagJOYCAPSA *pjc, UINT size)
{
	if (!g_config.JoystickEmul) {
		UINT res = joyGetDevCaps(joy, pjc, size);
		if (g_config.InvertYAxis && joy == 0 && pjc && size == 0x194) joyYmax = pjc->wYmax;
		return res;
	}
	if (joy != 0) return MMSYSERR_NODRIVER;
	if (size != 0x194) return MMSYSERR_INVALPARAM;
	memset(pjc, 0, size);
	if (g_config.JoystickEmul == 2) {
		pjc->wXmax = 65536;
		pjc->wYmax = 65535;
		pjc->wZmax = 255;
		pjc->wRmax = 65536;
		pjc->wUmax = 65536;
		pjc->wVmax = 255;
		pjc->wNumButtons = 14;
		pjc->wMaxButtons = 14;
		pjc->wNumAxes = 6;
		pjc->wMaxAxes = 6;
		pjc->wCaps = JOYCAPS_HASZ | JOYCAPS_HASR | JOYCAPS_HASU | JOYCAPS_HASV | JOYCAPS_HASPOV | JOYCAPS_POV4DIR;
		return JOYERR_NOERROR;
	}
	pjc->wXmax = 512;
	pjc->wYmax = 512;
	pjc->wNumButtons = 5;
	pjc->wMaxButtons = 5;
	pjc->wNumAxes = 2;
	pjc->wMaxAxes = 2;
	return JOYERR_NOERROR;
}

static DWORD lastGetPos;

static const DWORD povmap[16] = {
	JOY_POVCENTERED, // nothing
	JOY_POVFORWARD, // DPAD_UP
	JOY_POVBACKWARD, // DPAD_DOWN
	JOY_POVCENTERED, // up and down
	JOY_POVLEFT, // DPAD_LEFT
	(270 + 45) * 100, // left and up
	(180 + 45) * 100, // down and left
	JOY_POVLEFT, // left and up and down
	JOY_POVRIGHT, // DPAD_RIGHT
	45 * 100, // up and right
	(90 + 45) * 100, // right and down
	JOY_POVRIGHT, // right and up and down

	// As from start, but  with both right and left pressed
	JOY_POVCENTERED, // nothing
	JOY_POVFORWARD, // DPAD_UP
	JOY_POVBACKWARD, // DPAD_DOWN
	JOY_POVCENTERED, // up and down
};

UINT WINAPI emulJoyGetPosEx(UINT joy, struct joyinfoex_tag *pji)
{
	if (!g_config.JoystickEmul) {
		UINT res = joyGetPosEx(joy, pji);
		if (g_config.InvertYAxis && joyYmax > 0) pji->dwYpos = joyYmax - pji->dwYpos;
		return res;
	}
	if (joy != 0) return MMSYSERR_NODRIVER;
	if (pji->dwSize != 0x34) return MMSYSERR_INVALPARAM;
	if (g_config.JoystickEmul == 2) {
		XINPUT_STATE state;
		XInputGetState(0, &state);
		pji->dwFlags = JOY_RETURNALL;
		pji->dwXpos = state.Gamepad.sThumbLX + 32768;
		pji->dwYpos = state.Gamepad.sThumbLY + 32768;
		if (!g_config.InvertYAxis) pji->dwYpos = 65536 - pji->dwYpos;
		// The 65536 value breaks XWA with in-game invert Y axis option
		pji->dwYpos = std::min(pji->dwYpos, DWORD(65535));
		pji->dwZpos = state.Gamepad.bRightTrigger;
		pji->dwRpos = state.Gamepad.sThumbRX + 32768;
		pji->dwUpos = state.Gamepad.sThumbRY + 32768;
		pji->dwVpos = state.Gamepad.bLeftTrigger;
		pji->dwButtons = 0;
		// Order matches XBox One controller joystick emulation default
		// A, B, X, Y first
		pji->dwButtons |= (state.Gamepad.wButtons & 0xf000) >> 12;
		// Shoulder buttons next
		pji->dwButtons |= (state.Gamepad.wButtons & 0x300) >> 4;
		// start and back, for some reason in flipped order compared to XINPUT
		pji->dwButtons |= (state.Gamepad.wButtons & 0x10) << 3;
		pji->dwButtons |= (state.Gamepad.wButtons & 0x20) << 1;
		// Thumb buttons
		pji->dwButtons |= (state.Gamepad.wButtons & 0xc0) << 2;
		// Triggers last, they are not mapped in the joystick emulation
		if (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) pji->dwButtons |= 0x400;
		if (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) pji->dwButtons |= 0x800;
		// These are not defined by XINPUT, and can't be remapped by the
		// XWA user interface as they will be buttons above 12, but map them just in case
		pji->dwButtons |= (state.Gamepad.wButtons & 0xc00) << 2;

		// XWA can map only 12 buttons, so map dpad to POV
		pji->dwPOV = povmap[state.Gamepad.wButtons & 15];

		pji->dwButtonNumber = 0;
		for (int i = 0; i < 32; i++)
		{
			if ((pji->dwButtons >> i) & 1) ++pji->dwButtonNumber;
		}
		return JOYERR_NOERROR;
	}
	DWORD now = GetTickCount();
	// Assume we started a new game
	if ((now - lastGetPos) > 5000)
	{
		SetCursorPos(240, 240);
		GetAsyncKeyState(VK_LBUTTON);
		GetAsyncKeyState(VK_RBUTTON);
		GetAsyncKeyState(VK_MBUTTON);
		GetAsyncKeyState(VK_XBUTTON1);
		GetAsyncKeyState(VK_XBUTTON2);
	}
	lastGetPos = now;
	POINT pos;
	GetCursorPos(&pos);
	pji->dwXpos = static_cast<DWORD>(std::min(256.0f + (pos.x - 240.0f) * g_config.MouseSensitivity, 512.0f));
	pji->dwYpos = static_cast<DWORD>(std::min(256.0f + (pos.y - 240.0f) * g_config.MouseSensitivity, 512.0f));
	pji->dwButtons = 0;
	pji->dwButtonNumber = 0;
	if (GetAsyncKeyState(VK_LBUTTON)) {
		pji->dwButtons |= 1;
		++pji->dwButtonNumber;
	}
	if (GetAsyncKeyState(VK_RBUTTON)) {
		pji->dwButtons |= 2;
		++pji->dwButtonNumber;
	}
	if (GetAsyncKeyState(VK_MBUTTON)) {
		pji->dwButtons |= 4;
		++pji->dwButtonNumber;
	}
	if (GetAsyncKeyState(VK_XBUTTON1)) {
		pji->dwButtons |= 8;
		++pji->dwButtonNumber;
	}
	if (GetAsyncKeyState(VK_XBUTTON2)) {
		pji->dwButtons |= 16;
		++pji->dwButtonNumber;
	}
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		pji->dwXpos = static_cast<DWORD>(std::max(256 - 256 * g_config.KbdSensitivity, 0.0f));
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		pji->dwXpos = static_cast<DWORD>(std::min(256 + 256 * g_config.KbdSensitivity, 512.0f));
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
		pji->dwYpos = static_cast<DWORD>(std::max(256 - 256 * g_config.KbdSensitivity, 0.0f));
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000) {
		pji->dwYpos = static_cast<DWORD>(std::min(256 + 256 * g_config.KbdSensitivity, 512.0f));
	}
	if (g_config.InvertYAxis) pji->dwYpos = 512 - pji->dwYpos;
	return JOYERR_NOERROR;
}
