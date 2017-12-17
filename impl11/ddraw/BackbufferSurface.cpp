// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include <emmintrin.h>
#include "DeviceResources.h"
#include "BackbufferSurface.h"
#include "Direct3DDevice.h"
#include "FrontbufferSurface.h"
#include "OffscreenSurface.h"
#include "DepthSurface.h"

BackbufferSurface::BackbufferSurface(DeviceResources* deviceResources)
{
	this->_refCount = 1;
	this->_deviceResources = deviceResources;

	this->_bufferSize = this->_deviceResources->_displayHeight * this->_deviceResources->_displayWidth * this->_deviceResources->_displayBpp;
	this->_buffer = new char[this->_bufferSize];
	memset(this->_buffer, 0, this->_bufferSize);
	this->_lockCount = 0;
}

BackbufferSurface::~BackbufferSurface()
{
	if (this->_deviceResources->_backbufferSurface == this)
	{
		this->_deviceResources->_backbufferSurface = nullptr;
	}

	delete[] this->_buffer;
}

HRESULT BackbufferSurface::QueryInterface(
	REFIID riid,
	LPVOID* obp
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;

	if (riid == IID_IDirect3DHALDevice)
	{
		str << " IDirect3DHALDevice";
	}
	else
	{
		str << " " << tostr_IID(riid);
	}

	LogText(str.str());
#endif

	if (riid == IID_IDirect3DHALDevice)
	{
		*obp = new Direct3DDevice(this->_deviceResources);

#if LOGGER
		str.str("");
		str << "\tDirect3DDevice\t" << *obp;
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

ULONG BackbufferSurface::AddRef()
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

ULONG BackbufferSurface::Release()
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

HRESULT BackbufferSurface::AddAttachedSurface(
	LPDIRECTDRAWSURFACE lpDDSAttachedSurface
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpDDSAttachedSurface == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	if (lpDDSAttachedSurface == (LPDIRECTDRAWSURFACE)this->_deviceResources->_depthSurface)
	{
#if LOGGER
		str.str("\tDD_OK");
		LogText(str.str());
#endif

		this->_deviceResources->_depthSurface->AddRef();

		return DD_OK;
	}

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT BackbufferSurface::AddOverlayDirtyRect(
	LPRECT lpRect
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

HRESULT BackbufferSurface::Blt(
	LPRECT lpDestRect,
	LPDIRECTDRAWSURFACE lpDDSrcSurface,
	LPRECT lpSrcRect,
	DWORD dwFlags,
	LPDDBLTFX lpDDBltFx
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	str << tostr_RECT(lpDestRect);
	str << " " << lpDDSrcSurface;
	str << tostr_RECT(lpSrcRect);
	str << " " << dwFlags;

	if ((dwFlags & DDBLT_COLORFILL) != 0 && lpDDBltFx != nullptr)
	{
		str << " " << (void*)lpDDBltFx->dwFillColor;
	}

	if ((dwFlags & DDBLT_ROP) != 0)
	{
		str << " ROP:" << lpDDBltFx->dwROP;
	}

	LogText(str.str());
#endif

	if (dwFlags & DDBLT_COLORFILL)
	{
		if (lpDDBltFx == nullptr)
		{
			return DDERR_INVALIDPARAMS;
		}

		unsigned short color = (unsigned short)lpDDBltFx->dwFillColor;
		if (color != 0 && this->_deviceResources->_displayBpp == 2)
		{
			unsigned short* buffer = (unsigned short*)this->_buffer;
			int length = this->_bufferSize / 2;

			int i;
			for (i = 32; i <= length; i += 32)
			{
				_mm_storeu_si128((__m128i *)(buffer + i - 32), _mm_set1_epi16(color));
				_mm_storeu_si128((__m128i *)(buffer + i - 24), _mm_set1_epi16(color));
				_mm_storeu_si128((__m128i *)(buffer + i - 16), _mm_set1_epi16(color));
				_mm_storeu_si128((__m128i *)(buffer + i - 8), _mm_set1_epi16(color));
			}
			i -= 32;
			for (; i < length; i++)
			{
				buffer[i] = color;
			}
		}
		else
		//{
		//	unsigned int* buffer = (unsigned int*)this->_buffer;
		//	int length = this->_bufferSize / 4;
		//	unsigned short color16 = (unsigned short)lpDDBltFx->dwFillColor;

		//	unsigned int red = (color16 & 0xF800) >> 8;
		//	unsigned int green = (color16 & 0x7E0) >> 3;
		//	unsigned int blue = (color16 & 0x1F) << 3;

		//	unsigned int color = (red << 16) | (green << 8) | blue;

		//	for (int i = 0; i < length; i++)
		//	{
		//		buffer[i] = color;
		//	}
		//}

		memset(this->_buffer, 0, this->_bufferSize);

		this->_deviceResources->clearColorSet = true;

		return DD_OK;
	}

	if (lpDestRect->right - lpDestRect->left == lpSrcRect->right - lpSrcRect->left &&
		lpDestRect->bottom - lpDestRect->top == lpSrcRect->bottom - lpSrcRect->top &&
		dwFlags == DDBLT_ROP && lpDDBltFx->dwROP == SRCCOPY)
	{
		BltFast(lpDestRect->left, lpDestRect->top, lpDDSrcSurface, lpSrcRect, DDBLTFAST_WAIT);
		return DD_OK;
	}

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT BackbufferSurface::BltBatch(
	LPDDBLTBATCH lpDDBltBatch,
	DWORD dwCount,
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

HRESULT BackbufferSurface::BltFast(
	DWORD dwX,
	DWORD dwY,
	LPDIRECTDRAWSURFACE lpDDSrcSurface,
	LPRECT lpSrcRect,
	DWORD dwTrans
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	str << " " << dwX << " " << dwY;

	if (lpDDSrcSurface == nullptr)
	{
		str << " NULL";
	}
	else if (lpDDSrcSurface == this->_deviceResources->_frontbufferSurface)
	{
		str << " FrontbufferSurface";
	}
	else if (lpDDSrcSurface == this->_deviceResources->_offscreenSurface)
	{
		str << " OffscreenSurface";
	}
	else
	{
		str << " " << lpDDSrcSurface;
	}

	str << tostr_RECT(lpSrcRect);

	if (dwTrans & DDBLTFAST_SRCCOLORKEY)
	{
		str << " SRCCOLORKEY";
	}

	if (dwTrans & DDBLTFAST_DESTCOLORKEY)
	{
		str << " DESTCOLORKEY";
	}

	if (dwTrans & DDBLTFAST_WAIT)
	{
		str << " WAIT";
	}

	if (dwTrans & DDBLTFAST_DONOTWAIT)
	{
		str << " DONOTWAIT";
	}

	LogText(str.str());
#endif

	if (lpDDSrcSurface != nullptr)
	{
		DWORD srcBpp = this->_deviceResources->_displayBpp == 1 ? 1 : 2;
		if (lpDDSrcSurface == this->_deviceResources->_frontbufferSurface)
		{
			copySurface(this->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, this->_deviceResources->_displayBpp, this->_deviceResources->_frontbufferSurface->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, srcBpp, dwX, dwY, lpSrcRect, (dwTrans & DDBLTFAST_SRCCOLORKEY) != 0);
			this->_deviceResources->_frontbufferSurface->wasBltFastCalled = true;
			return DD_OK;
		}

		if (lpDDSrcSurface == this->_deviceResources->_offscreenSurface)
		{
			copySurface(this->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, this->_deviceResources->_displayBpp, this->_deviceResources->_offscreenSurface->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, srcBpp, dwX, dwY, lpSrcRect, (dwTrans & DDBLTFAST_SRCCOLORKEY) != 0);
			return DD_OK;
		}
	}

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT BackbufferSurface::DeleteAttachedSurface(
	DWORD dwFlags,
	LPDIRECTDRAWSURFACE lpDDSAttachedSurface
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpDDSAttachedSurface == nullptr)
	{
#if LOGGER
		str.str("\tDD_OK");
		LogText(str.str());
#endif

		return DD_OK;
	}

	if (lpDDSAttachedSurface == (LPDIRECTDRAWSURFACE)this->_deviceResources->_depthSurface)
	{
#if LOGGER
		str.str("\tDD_OK");
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

HRESULT BackbufferSurface::EnumAttachedSurfaces(
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

HRESULT BackbufferSurface::EnumOverlayZOrders(
	DWORD dwFlags,
	LPVOID lpContext,
	LPDDENUMSURFACESCALLBACK lpfnCallback
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

HRESULT BackbufferSurface::Flip(
	LPDIRECTDRAWSURFACE lpDDSurfaceTargetOverride,
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

HRESULT BackbufferSurface::GetAttachedSurface(
	LPDDSCAPS lpDDSCaps,
	LPDIRECTDRAWSURFACE FAR *lplpDDAttachedSurface
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__ << " " << lpDDSCaps->dwCaps;
	LogText(str.str());
#endif

	if (lpDDSCaps->dwCaps == DDSCAPS_ZBUFFER && this->_deviceResources->_depthSurface)
	{
		if (lplpDDAttachedSurface)
		{
			*lplpDDAttachedSurface = (LPDIRECTDRAWSURFACE)this->_deviceResources->_depthSurface;
		}

#if LOGGER
		str.str("\tDD_OK");
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

HRESULT BackbufferSurface::GetBltStatus(
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

HRESULT BackbufferSurface::GetCaps(
	LPDDSCAPS lpDDSCaps
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

HRESULT BackbufferSurface::GetClipper(
	LPDIRECTDRAWCLIPPER FAR *lplpDDClipper
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

HRESULT BackbufferSurface::GetColorKey(
	DWORD dwFlags,
	LPDDCOLORKEY lpDDColorKey
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

HRESULT BackbufferSurface::GetDC(
	HDC FAR *lphDC
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

HRESULT BackbufferSurface::GetFlipStatus(
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

HRESULT BackbufferSurface::GetOverlayPosition(
	LPLONG lplX,
	LPLONG lplY
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

HRESULT BackbufferSurface::GetPalette(
	LPDIRECTDRAWPALETTE FAR *lplpDDPalette
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

HRESULT BackbufferSurface::GetPixelFormat(
	LPDDPIXELFORMAT lpDDPixelFormat
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

HRESULT BackbufferSurface::GetSurfaceDesc(
	LPDDSURFACEDESC lpDDSurfaceDesc
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
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

	this->_deviceResources->DefaultSurfaceDesc(lpDDSurfaceDesc, DDSCAPS_BACKBUFFER | DDSCAPS_VIDEOMEMORY);

	if (g_config.XWAMode && !this->_deviceResources->_frontbufferSurface)
	{
		// workaround for XWA with 32-bit patch applied,
		// this code is used for the star field which is
		// being rendered in 32-bit.
		lpDDSurfaceDesc->lPitch = this->_deviceResources->_displayWidth * this->_deviceResources->_displayBpp;
	}

#if LOGGER
	str.str("");
	str << "\t" << tostr_DDSURFACEDESC(lpDDSurfaceDesc);
	LogText(str.str());
#endif

	return DD_OK;
}

HRESULT BackbufferSurface::Initialize(
	LPDIRECTDRAW lpDD,
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

HRESULT BackbufferSurface::IsLost()
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

HRESULT BackbufferSurface::Lock(
	LPRECT lpDestRect,
	LPDDSURFACEDESC lpDDSurfaceDesc,
	DWORD dwFlags,
	HANDLE hEvent
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	str << tostr_RECT(lpDestRect);
	LogText(str.str());
#endif

	++_lockCount;
	if (lpDDSurfaceDesc == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	if (lpDestRect == nullptr)
	{
		this->_deviceResources->DefaultSurfaceDesc(lpDDSurfaceDesc, DDSCAPS_BACKBUFFER | DDSCAPS_VIDEOMEMORY);
		lpDDSurfaceDesc->dwFlags |= DDSD_LPSURFACE;
		lpDDSurfaceDesc->lpSurface = this->_buffer;

		if (g_config.XWAMode && this->_deviceResources->_frontbufferSurface != nullptr)
		{
			memset(this->_buffer, 0x80, this->_bufferSize);
			lpDDSurfaceDesc->lPitch = this->_deviceResources->_displayWidth * 2;
		}
		else
		{
			if (this->_deviceResources->inScene)
			{
				if (!this->_deviceResources->inSceneBackbufferLocked)
				{
					memset(this->_buffer, 0, this->_bufferSize);
				}

				this->_deviceResources->inSceneBackbufferLocked = true;
			}

			if (g_config.XWAMode && !this->_deviceResources->inScene && this->_deviceResources->sceneRenderedEmpty)
			{
				this->_deviceResources->sceneRenderedEmpty = false;

#if LOGGER
				str.str("\tSCREENSHOT flight");
				LogText(str.str());
#endif

				this->_deviceResources->RetrieveBackBuffer(this->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, this->_deviceResources->_displayBpp);
			}
		}

		return DD_OK;
	}

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT BackbufferSurface::ReleaseDC(
	HDC hDC
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

HRESULT BackbufferSurface::Restore()
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

HRESULT BackbufferSurface::SetClipper(
	LPDIRECTDRAWCLIPPER lpDDClipper
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

HRESULT BackbufferSurface::SetColorKey(
	DWORD dwFlags,
	LPDDCOLORKEY lpDDColorKey
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

HRESULT BackbufferSurface::SetOverlayPosition(
	LONG lX,
	LONG lY
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

HRESULT BackbufferSurface::SetPalette(
	LPDIRECTDRAWPALETTE lpDDPalette
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

HRESULT BackbufferSurface::Unlock(
	LPVOID lpSurfaceData
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	// Fixup briefing text when xwahacker was used to force _displayBpp == 4
	if (g_config.XWAMode && this->_deviceResources->_frontbufferSurface != nullptr && this->_deviceResources->_displayBpp == 4)
	{
		int length = this->_deviceResources->_displayWidth * this->_deviceResources->_displayHeight;

		unsigned short* buffer16 = (unsigned short*)this->_buffer;
		unsigned int* buffer32 = (unsigned int*)this->_buffer;

		for (int i = length - 1; i >= 0; i--)
		{
			unsigned short color16 = buffer16[i];

			buffer32[i] = convertColorB5G6R5toB8G8R8X8(color16);
		}
	}

	if (_lockCount == 0) return DDERR_NOTLOCKED;
	--_lockCount;
	return DD_OK;
}

HRESULT BackbufferSurface::UpdateOverlay(
	LPRECT lpSrcRect,
	LPDIRECTDRAWSURFACE lpDDDestSurface,
	LPRECT lpDestRect,
	DWORD dwFlags,
	LPDDOVERLAYFX lpDDOverlayFx
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

HRESULT BackbufferSurface::UpdateOverlayDisplay(
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

HRESULT BackbufferSurface::UpdateOverlayZOrder(
	DWORD dwFlags,
	LPDIRECTDRAWSURFACE lpDDSReference
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
