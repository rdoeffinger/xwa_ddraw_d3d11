// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "DeviceResources.h"
#include "DirectDraw2.h"

DirectDraw2::DirectDraw2(DeviceResources* deviceResources)
{
	g_config.runAutopatch();
	this->_refCount = 1;
	this->_deviceResources = deviceResources;
}

DirectDraw2::~DirectDraw2()
{
}

HRESULT DirectDraw2::QueryInterface(
	REFIID riid,
	LPVOID* obp
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	str << " " << tostr_IID(riid);
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tE_NOINTERFACE");
	LogText(str.str());
#endif

	return E_NOINTERFACE;
}

ULONG DirectDraw2::AddRef()
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

ULONG DirectDraw2::Release()
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

HRESULT DirectDraw2::Compact()
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

HRESULT DirectDraw2::CreateClipper(
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

HRESULT DirectDraw2::CreatePalette(
	DWORD dwFlags,
	LPPALETTEENTRY lpColorTable,
	LPDIRECTDRAWPALETTE FAR *lplpDDPalette,
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

HRESULT DirectDraw2::CreateSurface(
	LPDDSURFACEDESC lpDDSurfaceDesc,
	LPDIRECTDRAWSURFACE FAR *lplpDDSurface,
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

HRESULT DirectDraw2::DuplicateSurface(
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

HRESULT DirectDraw2::EnumDisplayModes(
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

	if (lpEnumModesCallback == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	DDSURFACEDESC sd{};
	sd.dwSize = sizeof(DDSURFACEDESC);
	sd.dwFlags = DDSD_PIXELFORMAT | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PITCH;

	// 0565
	sd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	sd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	sd.ddpfPixelFormat.dwRGBBitCount = 16;
	sd.ddpfPixelFormat.dwRBitMask = 0xF800;
	sd.ddpfPixelFormat.dwGBitMask = 0x07E0;
	sd.ddpfPixelFormat.dwBBitMask = 0x001F;

	sd.dwWidth = 0x280;
	sd.dwHeight = 0x1E0;
	sd.lPitch = sd.dwWidth * 2;

	if (lpEnumModesCallback(&sd, lpContext) != DDENUMRET_OK)
		return DD_OK;

	sd.dwWidth = 0x320;
	sd.dwHeight = 0x258;
	sd.lPitch = sd.dwWidth * 2;

	if (lpEnumModesCallback(&sd, lpContext) != DDENUMRET_OK)
		return DD_OK;

	sd.dwWidth = 0x400;
	sd.dwHeight = 0x300;
	sd.lPitch = sd.dwWidth * 2;

	if (lpEnumModesCallback(&sd, lpContext) != DDENUMRET_OK)
		return DD_OK;

	sd.dwWidth = 0x480;
	sd.dwHeight = 0x360;
	sd.lPitch = sd.dwWidth * 2;

	if (lpEnumModesCallback(&sd, lpContext) != DDENUMRET_OK)
		return DD_OK;

	sd.dwWidth = 0x500;
	sd.dwHeight = 0x400;
	sd.lPitch = sd.dwWidth * 2;

	if (lpEnumModesCallback(&sd, lpContext) != DDENUMRET_OK)
		return DD_OK;

	sd.dwWidth = 0x640;
	sd.dwHeight = 0x4B0;
	sd.lPitch = sd.dwWidth * 2;

	if (lpEnumModesCallback(&sd, lpContext) != DDENUMRET_OK)
		return DD_OK;

	return DD_OK;
}

HRESULT DirectDraw2::EnumSurfaces(
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

HRESULT DirectDraw2::FlipToGDISurface()
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

HRESULT DirectDraw2::GetCaps(
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

HRESULT DirectDraw2::GetDisplayMode(
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

HRESULT DirectDraw2::GetFourCCCodes(
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

HRESULT DirectDraw2::GetGDISurface(
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

HRESULT DirectDraw2::GetMonitorFrequency(
	LPDWORD lpdwFrequency
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

HRESULT DirectDraw2::GetScanLine(
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

HRESULT DirectDraw2::GetVerticalBlankStatus(
	LPBOOL lpbIsInVB
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

HRESULT DirectDraw2::Initialize(
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

HRESULT DirectDraw2::RestoreDisplayMode()
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

HRESULT DirectDraw2::SetCooperativeLevel(
	HWND hWnd,
	DWORD dwFlags
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

HRESULT DirectDraw2::SetDisplayMode(
	DWORD dwWidth,
	DWORD dwHeight,
	DWORD dwBPP,
	DWORD dwRefreshRate,
	DWORD dwFlags
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

HRESULT DirectDraw2::WaitForVerticalBlank(
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

HRESULT DirectDraw2::GetAvailableVidMem(
	LPDDSCAPS lpDDSCaps,
	LPDWORD lpdwTotal,
	LPDWORD lpdwFree
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if ((lpdwTotal == nullptr) || (lpdwFree == nullptr))
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	ComPtr<IDXGIDevice> dxgiDevice;
	if (FAILED(this->_deviceResources->_d3dDevice.As(&dxgiDevice)))
		return DDERR_INVALIDOBJECT;

	ComPtr<IDXGIAdapter> dxgiAdapter;
	if (FAILED(dxgiDevice->GetAdapter(&dxgiAdapter)))
		return DDERR_INVALIDOBJECT;

	DXGI_ADAPTER_DESC desc;
	if (FAILED(dxgiAdapter->GetDesc(&desc)))
		return DDERR_INVALIDOBJECT;

	DWORD total = desc.DedicatedVideoMemory + desc.DedicatedSystemMemory + desc.SharedSystemMemory;

	*lpdwTotal = total;
	*lpdwFree = *lpdwTotal;

#if LOGGER
	str.str("");
	str << "\tTotal: " << *lpdwTotal << "\tFree: " << *lpdwFree;
	LogText(str.str());
#endif

	return DD_OK;
}
