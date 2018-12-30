// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "DeviceResources.h"
#include "DirectDraw.h"
#include "DirectDraw2.h"
#include "DirectDrawPalette.h"
#include "Direct3D.h"
#include "PrimarySurface.h"
#include "DepthSurface.h"
#include "BackbufferSurface.h"
#include "FrontbufferSurface.h"
#include "OffscreenSurface.h"
#include "TextureSurface.h"

DirectDraw::DirectDraw(DeviceResources* deviceResources)
{
	this->_refCount = 1;
	this->_deviceResources = deviceResources;
	this->_hWnd = NULL;
}

DirectDraw::~DirectDraw()
{
	delete this->_deviceResources;
	this->_deviceResources = nullptr;
}

HRESULT DirectDraw::QueryInterface(
	REFIID riid,
	LPVOID* obp
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;

	if (riid == IID_IDirectDraw2)
	{
		str << " IDirectDraw2";
	}
	else if (riid == IID_IDirectDraw4)
	{
		str << " IDirectDraw4";
	}
	else if (riid == IID_IDirect3D)
	{
		str << " IDirect3D";
	}
	else
	{
		str << " " << tostr_IID(riid);
	}

	LogText(str.str());
#endif

	if (obp == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	if (riid == IID_IDirectDraw2)
	{
		*obp = new DirectDraw2(this->_deviceResources);

#if LOGGER
		str.str("");
		str << "\tS_OK\t" << *obp;
		LogText(str.str());
#endif

		return S_OK;
	}

	if (riid == IID_IDirect3D)
	{
		*obp = new Direct3D(this->_deviceResources);

#if LOGGER
		str.str("");
		str << "\tS_OK\t" << *obp;
		LogText(str.str());
#endif

		return S_OK;
	}

#if LOGGER
	str.str("\tE_NOINTERFACE");
	LogText(str.str());
#endif

	return E_NOINTERFACE;
}

ULONG DirectDraw::AddRef()
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	this->_refCount++;

#if LOGGER
	str.str("");
	str << "\t" << this->_refCount;
	LogText(str.str());
#endif

	return this->_refCount;
}

ULONG DirectDraw::Release()
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	this->_refCount--;

#if LOGGER
	str.str("");
	str << "\t" << this->_refCount;
	LogText(str.str());
#endif

	if (this->_refCount == 0)
	{
		delete this;
		return 0;
	}

	return this->_refCount;
}

HRESULT DirectDraw::Compact()
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::CreateClipper(
	DWORD dwFlags,
	LPDIRECTDRAWCLIPPER FAR *lplpDDClipper,
	IUnknown FAR *pUnkOuter
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::CreatePalette(
	DWORD dwFlags,
	LPPALETTEENTRY lpColorTable,
	LPDIRECTDRAWPALETTE FAR *lplpDDPalette,
	IUnknown FAR *pUnkOuter
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;

	if (dwFlags & DDPCAPS_1BIT)
	{
		str << " 1BIT";
	}

	if (dwFlags & DDPCAPS_2BIT)
	{
		str << " 2BIT";
	}

	if (dwFlags & DDPCAPS_4BIT)
	{
		str << " 4BIT";
	}

	if (dwFlags & DDPCAPS_8BITENTRIES)
	{
		str << " 8BITENTRIES";
	}

	if (dwFlags & DDPCAPS_8BIT)
	{
		str << " 8BIT";
	}

	if (dwFlags & DDPCAPS_ALLOW256)
	{
		str << " ALLOW256";
	}

	LogText(str.str());
#endif

	if (lplpDDPalette != nullptr)
	{
		*lplpDDPalette = new DirectDrawPalette();
		return DD_OK;
	}

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::CreateSurface(
	LPDDSURFACEDESC lpDDSurfaceDesc,
	LPDIRECTDRAWSURFACE FAR *lplpDDSurface,
	IUnknown FAR *pUnkOuter
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	str << tostr_DDSURFACEDESC(lpDDSurfaceDesc);
	LogText(str.str());
#endif

	if (lpDDSurfaceDesc == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
	{
		bool hasBackbufferAttached = (lpDDSurfaceDesc->ddsCaps.dwCaps&DDSCAPS_FLIP) != 0;

		PrimarySurface* primarySurface = new PrimarySurface(this->_deviceResources, hasBackbufferAttached);
		this->_deviceResources->_primarySurface = primarySurface;
		*lplpDDSurface = primarySurface;

#if LOGGER
		str.str("");
		str << "\tPrimarySurface\t" << *lplpDDSurface;
		LogText(str.str());
#endif

		return DD_OK;
	}

	if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_ZBUFFER)
	{
		DepthSurface* depthSurface = new DepthSurface(this->_deviceResources);
		this->_deviceResources->_depthSurface = depthSurface;
		*lplpDDSurface = depthSurface;

#if LOGGER
		str.str("");
		str << "\tDepthSurface\t" << *lplpDDSurface;
		LogText(str.str());
#endif

		return DD_OK;
	}

	if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_OFFSCREENPLAIN)
	{
		if (this->_deviceResources->_backbufferSurface == nullptr)
		{
			BackbufferSurface* backbufferSurface = new BackbufferSurface(this->_deviceResources);
			this->_deviceResources->_backbufferSurface = backbufferSurface;
			*lplpDDSurface = backbufferSurface;

#if LOGGER
			str.str("");
			str << "\tBackbufferSurface\t" << *lplpDDSurface;
			LogText(str.str());
#endif

			return DD_OK;
		}

		if (this->_deviceResources->_frontbufferSurface == nullptr)
		{
			FrontbufferSurface* frontbufferSurface = new FrontbufferSurface(this->_deviceResources);
			this->_deviceResources->_frontbufferSurface = frontbufferSurface;
			*lplpDDSurface = frontbufferSurface;

#if LOGGER
			str.str("");
			str << "\tFrontbufferSurface\t" << *lplpDDSurface;
			LogText(str.str());
#endif

			return DD_OK;
		}

		if (this->_deviceResources->_offscreenSurface == nullptr)
		{
			OffscreenSurface* offscreenSurface = new OffscreenSurface(this->_deviceResources);
			this->_deviceResources->_offscreenSurface = offscreenSurface;
			*lplpDDSurface = offscreenSurface;

#if LOGGER
			str.str("");
			str << "\tOffscreenSurface\t" << *lplpDDSurface;
			LogText(str.str());
#endif

			return DD_OK;
		}
	}

	if (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_TEXTURE)
	{
		bool allocOnLoad = (lpDDSurfaceDesc->ddsCaps.dwCaps & DDSCAPS_ALLOCONLOAD) != 0;
		DWORD mipmapCount = (lpDDSurfaceDesc->dwFlags & DDSD_MIPMAPCOUNT) != 0 ? lpDDSurfaceDesc->dwMipMapCount : 1;

		TextureSurface* textureSurface = new TextureSurface(this->_deviceResources, allocOnLoad, lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, lpDDSurfaceDesc->ddpfPixelFormat, mipmapCount);
		*lplpDDSurface = textureSurface;

#if LOGGER
		str.str("");
		str << "\tTextureSurface\t" << *lplpDDSurface;
		LogText(str.str());
#endif

		return DD_OK;
	}

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::DuplicateSurface(
	LPDIRECTDRAWSURFACE lpDDSurface,
	LPDIRECTDRAWSURFACE FAR *lplpDupDDSurface
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::EnumDisplayModes(
	DWORD dwFlags,
	LPDDSURFACEDESC lpDDSurfaceDesc,
	LPVOID lpContext,
	LPDDENUMMODESCALLBACK lpEnumModesCallback
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::EnumSurfaces(
	DWORD dwFlags,
	LPDDSURFACEDESC lpDDSD,
	LPVOID lpContext,
	LPDDENUMSURFACESCALLBACK lpEnumSurfacesCallback
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::FlipToGDISurface()
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::GetCaps(
	LPDDCAPS lpDDDriverCaps,
	LPDDCAPS lpDDHELCaps
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::GetDisplayMode(
	LPDDSURFACEDESC lpDDSurfaceDesc
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::GetFourCCCodes(
	LPDWORD lpNumCodes,
	LPDWORD lpCodes
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::GetGDISurface(
	LPDIRECTDRAWSURFACE FAR *lplpGDIDDSSurface
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::GetMonitorFrequency(
	LPDWORD lpdwFrequency
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpdwFrequency == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	*lpdwFrequency = 24;

	return DD_OK;
}

HRESULT DirectDraw::GetScanLine(
	LPDWORD lpdwScanLine
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

static bool vblank;

HRESULT DirectDraw::GetVerticalBlankStatus(
	LPBOOL lpbIsInVB
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_CURRENTLYNOTAVAIL");
	LogText(str.str());
#endif

	if (!_deviceResources || !_deviceResources->_output)
		return DDERR_UNSUPPORTED;
	*lpbIsInVB = vblank;
	if (vblank && _deviceResources->_output->WaitForVBlank() != S_OK)
		return DDERR_UNSUPPORTED;
	vblank = !vblank;

	return DD_OK;
}

HRESULT DirectDraw::Initialize(
	GUID FAR *lpGUID
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT DirectDraw::RestoreDisplayMode()
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDD_OK");
	LogText(str.str());
#endif

	return DD_OK;
}

static WNDPROC savedWndProc;

LRESULT WINAPI WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// All non-mouse/kbd events
	if (!(Msg >= WM_MOUSEFIRST && Msg <= WM_MOUSELAST) && !(Msg >= WM_KEYFIRST && Msg <= WM_KEYLAST)) {
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}
	// All mouse events outside the window
	if (Msg >= WM_MOUSEFIRST && Msg <= WM_MOUSELAST) {
		RECT r;
		GetClientRect(hWnd, &r);
		int x = lParam & 0xffff;
		int y = (unsigned)lParam >> 16;
		if (x >= r.right || y >= r.bottom) {
			ReleaseCapture();
			return DefWindowProc(hWnd, Msg, wParam, lParam);
		}
	}
	return savedWndProc(hWnd, Msg, wParam, lParam);
}

HRESULT DirectDraw::SetCooperativeLevel(
	HWND hWnd,
	DWORD dwFlags
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	str << " " << hWnd;

	if (dwFlags & DDSCL_FULLSCREEN)
	{
		str << " FULLSCREEN";
	}

	if (dwFlags & DDSCL_ALLOWREBOOT)
	{
		str << " ALLOWREBOOT";
	}

	if (dwFlags & DDSCL_NOWINDOWCHANGES)
	{
		str << " NOWINDOWCHANGES";
	}

	if (dwFlags & DDSCL_NORMAL)
	{
		str << " NORMAL";
	}

	if (dwFlags & DDSCL_EXCLUSIVE)
	{
		str << " EXCLUSIVE";
	}

	if (dwFlags & DDSCL_ALLOWMODEX)
	{
		str << " ALLOWMODEX";
	}

	if (dwFlags & DDSCL_SETFOCUSWINDOW)
	{
		str << " SETFOCUSWINDOW";
	}

	if (dwFlags & DDSCL_SETDEVICEWINDOW)
	{
		str << " SETDEVICEWINDOW";
	}

	if (dwFlags & DDSCL_CREATEDEVICEWINDOW)
	{
		str << " CREATEDEVICEWINDOW";
	}

	LogText(str.str());
#endif

	this->_hWnd = hWnd;

	if (g_config.Fullscreen == -1 && hWnd != nullptr)
	{
		ShowCursor(TRUE);
		LONG old = GetWindowLong(hWnd, GWL_STYLE);
		SetWindowLong(hWnd, GWL_STYLE, old | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_OVERLAPPEDWINDOW);
		LONG oldWndProc = GetWindowLong(hWnd, GWL_WNDPROC);
		if (oldWndProc != (LONG)WndProc) savedWndProc = (WNDPROC)oldWndProc;
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)WndProc);
		SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

#if LOGGER
	str.str("\tDD_OK");
	LogText(str.str());
#endif

	return DD_OK;
}

HRESULT DirectDraw::SetDisplayMode(
	DWORD dwWidth,
	DWORD dwHeight,
	DWORD dwBPP
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	str << " " << dwWidth << "x" << dwHeight << " " << dwBPP;

	LogText(str.str());
#endif

	this->_deviceResources->_displayWidth = dwWidth;
	this->_deviceResources->_displayHeight = dwHeight;
	this->_deviceResources->_displayBpp = dwBPP / 8;

	if (FAILED(this->_deviceResources->OnSizeChanged(this->_hWnd, dwWidth, dwHeight)))
	{
#if LOGGER
		str.str("\tDDERR_INVALIDOBJECT");
		LogText(str.str());
#endif

		return DDERR_INVALIDOBJECT;
	}

#if LOGGER
	str.str("\tDD_OK");
	LogText(str.str());
#endif

	return DD_OK;
}

HRESULT DirectDraw::WaitForVerticalBlank(
	DWORD dwFlags,
	HANDLE hEvent
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}
