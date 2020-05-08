// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "DeviceResources.h"
#include "DirectDrawPalette.h"
#include "PrimarySurface.h"
#include "BackbufferSurface.h"
#include "FrontbufferSurface.h"
#include "XwaDrawTextHook.h"
#include "XwaDrawRadarHook.h"
#include "XwaDrawBracketHook.h"

// defining NOMINMAX unfortunately breaks compilation of some DX headers
#undef min
#undef max
#include <algorithm>

PrimarySurface::PrimarySurface(DeviceResources* deviceResources, bool hasBackbufferAttached)
{
	this->_refCount = 1;
	this->_deviceResources = deviceResources;

	this->_hasBackbufferAttached = hasBackbufferAttached;

	if (this->_hasBackbufferAttached)
	{
		*this->_backbufferSurface.GetAddressOf() = new BackbufferSurface(this->_deviceResources);
		this->_deviceResources->_backbufferSurface = this->_backbufferSurface;
	}

	this->_flipFrames = 0;
	palette = nullptr;
}

PrimarySurface::~PrimarySurface()
{
	if (this->_hasBackbufferAttached)
	{
		for (ULONG ref = this->_backbufferSurface->AddRef(); ref > 1; ref--)
		{
			this->_backbufferSurface->Release();
		}
	}

	if (this->_deviceResources->_primarySurface == this)
	{
		this->_deviceResources->_primarySurface = nullptr;
	}
	if (palette) palette->Release();
	palette = nullptr;
}

HRESULT PrimarySurface::QueryInterface(
	REFIID riid,
	LPVOID* obp
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tE_NOINTERFACE");
	LogText(str.str());
#endif

	return E_NOINTERFACE;
}

ULONG PrimarySurface::AddRef()
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

ULONG PrimarySurface::Release()
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

HRESULT PrimarySurface::AddAttachedSurface(
	LPDIRECTDRAWSURFACE lpDDSAttachedSurface
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

HRESULT PrimarySurface::AddOverlayDirtyRect(
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

HRESULT PrimarySurface::Blt(
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

	if (lpDDSrcSurface == this->_deviceResources->_backbufferSurface)
	{
		str << " BackbufferSurface";
	}
	else
	{
		str << " " << lpDDSrcSurface;
	}

	str << tostr_RECT(lpSrcRect);

	if ((dwFlags & DDBLT_COLORFILL) != 0 && lpDDBltFx != nullptr)
	{
		str << " " << (void*)lpDDBltFx->dwFillColor;
	}

	LogText(str.str());
#endif

	if (dwFlags & DDBLT_COLORFILL)
	{
		if (lpDDBltFx == nullptr)
		{
			return DDERR_INVALIDPARAMS;
		}

		return DD_OK;
	}

	if (lpDDSrcSurface != nullptr)
	{
		if (lpDDSrcSurface == this->_deviceResources->_backbufferSurface)
		{
			return this->Flip(NULL, 0);
		}
	}

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT PrimarySurface::BltBatch(
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

HRESULT PrimarySurface::BltFast(
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
	else if (lpDDSrcSurface == this->_deviceResources->_backbufferSurface)
	{
		str << " BackbufferSurface";
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
		if (lpDDSrcSurface == this->_deviceResources->_backbufferSurface)
		{
			return this->Flip(this->_deviceResources->_backbufferSurface, 0);
		}
	}

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT PrimarySurface::DeleteAttachedSurface(
	DWORD dwFlags,
	LPDIRECTDRAWSURFACE lpDDSAttachedSurface
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

HRESULT PrimarySurface::EnumAttachedSurfaces(
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

HRESULT PrimarySurface::EnumOverlayZOrders(
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

HRESULT PrimarySurface::Flip(
	LPDIRECTDRAWSURFACE lpDDSurfaceTargetOverride,
	DWORD dwFlags
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;

	if (lpDDSurfaceTargetOverride == nullptr)
	{
		str << " NULL";
	}
	else if (lpDDSurfaceTargetOverride == this->_deviceResources->_backbufferSurface)
	{
		str << " BackbufferSurface";
	}
	else if (lpDDSurfaceTargetOverride == this->_deviceResources->_frontbufferSurface)
	{
		str << " FrontbufferSurface";
	}
	else
	{
		str << " " << lpDDSurfaceTargetOverride;
	}

	if (dwFlags & DDFLIP_WAIT)
	{
		str << " WAIT";
	}

	LogText(str.str());
#endif

	this->_deviceResources->sceneRenderedEmpty = this->_deviceResources->sceneRendered == false;
	this->_deviceResources->sceneRendered = false;

	if (this->_deviceResources->sceneRenderedEmpty)
	{
		this->_deviceResources->_d3dDeviceContext->ClearRenderTargetView(this->_deviceResources->_renderTargetView, this->_deviceResources->clearColor);
		this->_deviceResources->clearColorSet = false;
		this->_deviceResources->_d3dDeviceContext->ClearDepthStencilView(this->_deviceResources->_depthStencilView, D3D11_CLEAR_DEPTH, this->_deviceResources->clearDepth, 0);
		this->_deviceResources->clearDepthSet = false;
	}

	if (lpDDSurfaceTargetOverride != nullptr)
	{
		if (lpDDSurfaceTargetOverride == this->_deviceResources->_frontbufferSurface)
		{
			HRESULT hr;

			if (FAILED(hr = this->_deviceResources->_backbufferSurface->BltFast(0, 0, this->_deviceResources->_frontbufferSurface, nullptr, 0)))
				return hr;

			return this->Flip(this->_deviceResources->_backbufferSurface, 0);
		}

		if (lpDDSurfaceTargetOverride == this->_deviceResources->_backbufferSurface)
		{
			if (!g_config.XWAMode || this->_deviceResources->_frontbufferSurface == nullptr)
			{
				if (FAILED(this->_deviceResources->RenderMain(this->_deviceResources->_backbufferSurface->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, this->_deviceResources->_displayBpp)))
					return DDERR_GENERIC;
			}
			else
			{
				const unsigned short colorKey = 0x8080;

				int width = 640;
				int height = 480;

				int topBlack = 0;

				if (this->_deviceResources->_displayBpp == 2)
				{
					int w = this->_deviceResources->_displayWidth;
					int x = (this->_deviceResources->_displayWidth - width) / 2;
					int y = (this->_deviceResources->_displayHeight - height) / 2;
					unsigned short* buffer = (unsigned short*)this->_deviceResources->_backbufferSurface->_buffer + y * w + x;

					for (int i = 0; i < height / 2; i++)
					{
						if (buffer[i * w] != colorKey)
							break;

						topBlack++;
					}
				}
				else
				{
					const unsigned int colorKey32 = convertColorB5G6R5toB8G8R8X8(colorKey);

					int w = this->_deviceResources->_displayWidth;
					int x = (this->_deviceResources->_displayWidth - width) / 2;
					int y = (this->_deviceResources->_displayHeight - height) / 2;
					unsigned int* buffer = (unsigned int*)this->_deviceResources->_backbufferSurface->_buffer + y * w + x;

					for (int i = 0; i < height / 2; i++)
					{
						if (buffer[i * w] != colorKey32)
							break;

						topBlack++;
					}
				}

				if (topBlack < height / 2)
				{
					height -= topBlack * 2;
				}

				RECT rc;
				rc.left = (this->_deviceResources->_displayWidth - width) / 2;
				rc.top = (this->_deviceResources->_displayHeight - height) / 2;
				rc.right = rc.left + width;
				rc.bottom = rc.top + height;

				int length = width * height * this->_deviceResources->_displayBpp;
				char* buffer = new char[length];

				copySurface(buffer, width, height, this->_deviceResources->_displayBpp, this->_deviceResources->_backbufferSurface->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, this->_deviceResources->_displayBpp, 0, 0, &rc, false);

				HRESULT hr = this->_deviceResources->RenderMain(buffer, width, height, this->_deviceResources->_displayBpp);

				delete[] buffer;

				if (FAILED(hr))
					return DDERR_GENERIC;
			}

			HRESULT hr;

			if (this->_deviceResources->_swapChain)
			{
				UINT rate = 24 * this->_deviceResources->_refreshRate.Denominator;
				UINT numerator = this->_deviceResources->_refreshRate.Numerator + this->_flipFrames;

				UINT interval = numerator / rate;
				this->_flipFrames = numerator % rate;

				interval = std::max(interval, 1u);
				// Maximum allowed Present argument is 4.
				if (g_config.RefreshLimit == 1 && interval > 4) interval = 4;

				hr = DD_OK;

				for (UINT i = 0; i < (g_config.RefreshLimit == 2 ? interval : 1); i++)
				{
					this->_deviceResources->_d3dDeviceContext->ResolveSubresource(this->_deviceResources->_backBuffer, 0, this->_deviceResources->_offscreenBuffer, 0, DXGI_FORMAT_B8G8R8A8_UNORM);

					if (FAILED(hr = this->_deviceResources->_swapChain->Present(g_config.RefreshLimit == 1 ? interval : 1, 0)))
					{
						static bool messageShown = false;

						if (!messageShown)
						{
							MessageBox(nullptr, _com_error(hr).ErrorMessage(), __FUNCTION__, MB_ICONERROR);
						}

						messageShown = true;

						hr = DDERR_SURFACELOST;
						break;
					}
				}
			}
			else
			{
				hr = DD_OK;
			}

			if (this->_deviceResources->_frontbufferSurface != nullptr)
			{
				if (this->_deviceResources->_frontbufferSurface->wasBltFastCalled)
				{
					memcpy(this->_deviceResources->_frontbufferSurface->_buffer2, this->_deviceResources->_frontbufferSurface->_buffer, this->_deviceResources->_frontbufferSurface->_bufferSize);
				}

				this->_deviceResources->_frontbufferSurface->wasBltFastCalled = false;
			}

			if (g_config.PresentSleepTime >= 0) {
				Sleep(g_config.PresentSleepTime);
			}

			return hr;
		}
	}
	else
	{
		HRESULT hr;

		if (this->_deviceResources->_swapChain)
		{
			hr = DD_OK;

			// Draw pending backbuffer updates
			// Note this breaks XWA showing only a black screen
			// with some green spots, especially in hangar and when targetting
			// if transparent parts are not replaced with colorkey in
			// Direct3DExecuteBuffer::Lock
			this->_deviceResources->RenderMain(this->_deviceResources->_backbufferSurface->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, this->_deviceResources->_displayBpp);
			this->RenderBracket();
			this->RenderRadar();
			this->RenderText();

			this->_deviceResources->_d3dDeviceContext->ResolveSubresource(this->_deviceResources->_backBuffer, 0, this->_deviceResources->_offscreenBuffer, 0, DXGI_FORMAT_B8G8R8A8_UNORM);

			if (FAILED(hr = this->_deviceResources->_swapChain->Present(g_config.VSyncEnabled ? 1 : 0, 0)))
			{
				static bool messageShown = false;

				if (!messageShown)
				{
					MessageBox(nullptr, _com_error(hr).ErrorMessage(), __FUNCTION__, MB_ICONERROR);
				}

				messageShown = true;

				hr = DDERR_SURFACELOST;
			}
#if 0
			// This seems to fix 8 bit rendering in X-Wing 95.
			// But not sure this is correct, and it's not exactly an important feature,
			// so do not enable it for now.
			DDBLTFX fx = {};
			fx.dwSize = sizeof(DDBLTFX);
			this->_deviceResources->_backbufferSurface->Blt(nullptr, nullptr, nullptr, DDBLT_COLORFILL, &fx);
#endif
		}
		else
		{
			hr = DD_OK;
		}

		if (g_config.PresentSleepTime >= 0) {
			Sleep(g_config.PresentSleepTime);
		}
		if (this->_deviceResources->_backbufferSurface) this->_deviceResources->_backbufferSurface->sawSrcCopy = false;
		return hr;
	}

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT PrimarySurface::GetAttachedSurface(
	LPDDSCAPS lpDDSCaps,
	LPDIRECTDRAWSURFACE FAR *lplpDDAttachedSurface
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpDDSCaps == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	if (lpDDSCaps->dwCaps & DDSCAPS_BACKBUFFER)
	{
		if (!this->_hasBackbufferAttached)
		{
#if LOGGER
			str.str("\tDDERR_INVALIDOBJECT");
			LogText(str.str());
#endif

			return DDERR_INVALIDOBJECT;
		}

		*lplpDDAttachedSurface = this->_backbufferSurface;
		this->_backbufferSurface->AddRef();

#if LOGGER
		str.str("");
		str << "\tBackbufferSurface\t" << *lplpDDAttachedSurface;
		LogText(str.str());
#endif

		return DD_OK;
	}

	return DDERR_UNSUPPORTED;
}

HRESULT PrimarySurface::GetBltStatus(
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

HRESULT PrimarySurface::GetCaps(
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

HRESULT PrimarySurface::GetClipper(
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

HRESULT PrimarySurface::GetColorKey(
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

HRESULT PrimarySurface::GetDC(
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

HRESULT PrimarySurface::GetFlipStatus(
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

HRESULT PrimarySurface::GetOverlayPosition(
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

HRESULT PrimarySurface::GetPalette(
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

HRESULT PrimarySurface::GetPixelFormat(
	LPDDPIXELFORMAT lpDDPixelFormat
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpDDPixelFormat)
	{
		lpDDPixelFormat->dwFlags = DDPF_RGB;
		lpDDPixelFormat->dwRGBBitCount = 16;
		lpDDPixelFormat->dwRBitMask = 0xF800;
		lpDDPixelFormat->dwGBitMask = 0x7E0;
		lpDDPixelFormat->dwBBitMask = 0x1F;
		return DD_OK;
	}

#if LOGGER
	str.str("\tDDERR_UNSUPPORTED");
	LogText(str.str());
#endif

	return DDERR_UNSUPPORTED;
}

HRESULT PrimarySurface::GetSurfaceDesc(
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

	this->_deviceResources->DefaultSurfaceDesc(lpDDSurfaceDesc, DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY);

#if LOGGER
	str.str("");
	str << "\t" << tostr_DDSURFACEDESC(lpDDSurfaceDesc);
	LogText(str.str());
#endif

	return DD_OK;
}

HRESULT PrimarySurface::Initialize(
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

HRESULT PrimarySurface::IsLost()
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

HRESULT PrimarySurface::Lock(
	LPRECT lpDestRect,
	LPDDSURFACEDESC lpDDSurfaceDesc,
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

HRESULT PrimarySurface::ReleaseDC(
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

HRESULT PrimarySurface::Restore()
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

HRESULT PrimarySurface::SetClipper(
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

HRESULT PrimarySurface::SetColorKey(
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

HRESULT PrimarySurface::SetOverlayPosition(
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

HRESULT PrimarySurface::SetPalette(
	LPDIRECTDRAWPALETTE lpDDPalette
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	DirectDrawPalette *pal = (DirectDrawPalette *)lpDDPalette;
	pal->AddRef();
	if (palette != nullptr) palette->Release();
	palette = pal;

	return DD_OK;
}

HRESULT PrimarySurface::Unlock(
	LPVOID lpSurfaceData
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

HRESULT PrimarySurface::UpdateOverlay(
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

HRESULT PrimarySurface::UpdateOverlayDisplay(
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

HRESULT PrimarySurface::UpdateOverlayZOrder(
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

void PrimarySurface::RenderText()
{
	if (g_xwa_text.empty()) return;
	this->_deviceResources->_d2d1RenderTarget->SaveDrawingState(this->_deviceResources->_d2d1DrawingStateBlock);
	this->_deviceResources->_d2d1RenderTarget->BeginDraw();

	UINT w;
	UINT h;

	if (g_config.AspectRatioPreserved)
	{
		if (this->_deviceResources->_backbufferHeight * this->_deviceResources->_displayWidth <= this->_deviceResources->_backbufferWidth * this->_deviceResources->_displayHeight)
		{
			w = this->_deviceResources->_backbufferHeight * this->_deviceResources->_displayWidth / this->_deviceResources->_displayHeight;
			h = this->_deviceResources->_backbufferHeight;
		}
		else
		{
			w = this->_deviceResources->_backbufferWidth;
			h = this->_deviceResources->_backbufferWidth * this->_deviceResources->_displayHeight / this->_deviceResources->_displayWidth;
		}
	}
	else
	{
		w = this->_deviceResources->_backbufferWidth;
		h = this->_deviceResources->_backbufferHeight;
	}

	UINT left = (this->_deviceResources->_backbufferWidth - w) / 2;
	UINT top = (this->_deviceResources->_backbufferHeight - h) / 2;

	float scaleX = (float)w / (float)this->_deviceResources->_displayWidth;
	float scaleY = (float)h / (float)this->_deviceResources->_displayHeight;

	ComPtr<IDWriteTextFormat> textFormats[3];
	int fontSizes[] = { 12, 16, 10 };

	for (int index = 0; index < 3; index++)
	{
		this->_deviceResources->_dwriteFactory->CreateTextFormat(
			g_config.TextFontFamily.c_str(),
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			(float)fontSizes[index] * std::min(scaleX, scaleY),
			L"en-US",
			&textFormats[index]);
	}

	ComPtr<ID2D1SolidColorBrush> brush;
	unsigned int brushColor = 0;

	IDWriteTextFormat* textFormat = nullptr;
	int fontSize = 0;

	for (const auto& xwaText : g_xwa_text)
	{
		if (xwaText.color != brushColor)
		{
			brushColor = xwaText.color;
			this->_deviceResources->_d2d1RenderTarget->CreateSolidColorBrush(D2D1::ColorF(brushColor), &brush);
		}

		if (xwaText.fontSize != fontSize)
		{
			fontSize = xwaText.fontSize;

			for (int index = 0; index < 3; index++)
			{
				if (fontSize == fontSizes[index])
				{
					textFormat = textFormats[index];
					break;
				}
			}
		}

		if (!brush)
		{
			continue;
		}

		if (!textFormat)
		{
			continue;
		}

		char t[2];
		t[0] = xwaText.textChar;
		t[1] = 0;

		std::wstring wtext = string_towstring(t);

		if (wtext.empty())
		{
			continue;
		}

		float x = (float)left + (float)xwaText.positionX * scaleX;
		float y = (float)top + (float)xwaText.positionY * scaleY;

		this->_deviceResources->_d2d1RenderTarget->DrawTextA(
			wtext.c_str(),
			wtext.length(),
			textFormat,
			D2D1::RectF(x, y, (float)this->_deviceResources->_backbufferWidth, (float)this->_deviceResources->_backbufferHeight),
			brush);
	}

	this->_deviceResources->_d2d1RenderTarget->EndDraw();
	this->_deviceResources->_d2d1RenderTarget->RestoreDrawingState(this->_deviceResources->_d2d1DrawingStateBlock);

	g_xwa_text.clear();
	g_xwa_text.reserve(4096);
}

void PrimarySurface::RenderRadar()
{
	if (g_xwa_radar.empty()) return;
	this->_deviceResources->_d2d1RenderTarget->SaveDrawingState(this->_deviceResources->_d2d1DrawingStateBlock);
	this->_deviceResources->_d2d1RenderTarget->BeginDraw();

	UINT w;
	UINT h;

	if (g_config.AspectRatioPreserved)
	{
		if (this->_deviceResources->_backbufferHeight * this->_deviceResources->_displayWidth <= this->_deviceResources->_backbufferWidth * this->_deviceResources->_displayHeight)
		{
			w = this->_deviceResources->_backbufferHeight * this->_deviceResources->_displayWidth / this->_deviceResources->_displayHeight;
			h = this->_deviceResources->_backbufferHeight;
		}
		else
		{
			w = this->_deviceResources->_backbufferWidth;
			h = this->_deviceResources->_backbufferWidth * this->_deviceResources->_displayHeight / this->_deviceResources->_displayWidth;
		}
	}
	else
	{
		w = this->_deviceResources->_backbufferWidth;
		h = this->_deviceResources->_backbufferHeight;
	}

	UINT left = (this->_deviceResources->_backbufferWidth - w) / 2;
	UINT top = (this->_deviceResources->_backbufferHeight - h) / 2;

	float scaleX = (float)w / (float)this->_deviceResources->_displayWidth;
	float scaleY = (float)h / (float)this->_deviceResources->_displayHeight;

	ComPtr<ID2D1SolidColorBrush> brush;
	unsigned int brushColor = 0;
	this->_deviceResources->_d2d1RenderTarget->CreateSolidColorBrush(D2D1::ColorF(brushColor), &brush);

	for (const auto& xwaRadar : g_xwa_radar)
	{
		unsigned short si = ((unsigned short*)0x08D9420)[xwaRadar.colorIndex];
		unsigned int esi;

		if (((bool(*)())0x0050DC50)() != 0)
		{
			unsigned short eax = si & 0x001F;
			unsigned short ecx = si & 0x7C00;
			unsigned short edx = si & 0x03E0;

			esi = (eax << 3) | (edx << 6) | (ecx << 9);
		}
		else
		{
			unsigned short eax = si & 0x001F;
			unsigned short edx = si & 0xF800;
			unsigned short ecx = si & 0x07E0;

			esi = (eax << 3) | (ecx << 5) | (edx << 8);
		}

		if (esi != brushColor)
		{
			brushColor = esi;
			this->_deviceResources->_d2d1RenderTarget->CreateSolidColorBrush(D2D1::ColorF(brushColor), &brush);
		}

		float x = left + (float)xwaRadar.positionX * scaleX;
		float y = top + (float)xwaRadar.positionY * scaleY;

		float deltaX = 2.0f * scaleX;
		float deltaY = 2.0f * scaleY;

		this->_deviceResources->_d2d1RenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), deltaX, deltaY), brush);
	}

	if (g_xwa_radar_selected_positionX != -1 && g_xwa_radar_selected_positionY != -1)
	{
		this->_deviceResources->_d2d1RenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &brush);

		float x = left + (float)g_xwa_radar_selected_positionX * scaleX;
		float y = top + (float)g_xwa_radar_selected_positionY * scaleY;

		float deltaX = 4.0f * scaleX;
		float deltaY = 4.0f * scaleY;

		this->_deviceResources->_d2d1RenderTarget->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x, y), deltaX, deltaY), brush);
	}

	this->_deviceResources->_d2d1RenderTarget->EndDraw();
	this->_deviceResources->_d2d1RenderTarget->RestoreDrawingState(this->_deviceResources->_d2d1DrawingStateBlock);

	g_xwa_radar.clear();
	g_xwa_radar_selected_positionX = -1;
	g_xwa_radar_selected_positionY = -1;
}

void PrimarySurface::RenderBracket()
{
	if (g_xwa_bracket.empty()) return;
	this->_deviceResources->_d2d1RenderTarget->SaveDrawingState(this->_deviceResources->_d2d1DrawingStateBlock);
	this->_deviceResources->_d2d1RenderTarget->BeginDraw();

	UINT w;
	UINT h;

	if (g_config.AspectRatioPreserved)
	{
		if (this->_deviceResources->_backbufferHeight * this->_deviceResources->_displayWidth <= this->_deviceResources->_backbufferWidth * this->_deviceResources->_displayHeight)
		{
			w = this->_deviceResources->_backbufferHeight * this->_deviceResources->_displayWidth / this->_deviceResources->_displayHeight;
			h = this->_deviceResources->_backbufferHeight;
		}
		else
		{
			w = this->_deviceResources->_backbufferWidth;
			h = this->_deviceResources->_backbufferWidth * this->_deviceResources->_displayHeight / this->_deviceResources->_displayWidth;
		}
	}
	else
	{
		w = this->_deviceResources->_backbufferWidth;
		h = this->_deviceResources->_backbufferHeight;
	}

	UINT left = (this->_deviceResources->_backbufferWidth - w) / 2;
	UINT top = (this->_deviceResources->_backbufferHeight - h) / 2;

	float scaleX = (float)w / (float)this->_deviceResources->_displayWidth;
	float scaleY = (float)h / (float)this->_deviceResources->_displayHeight;

	ComPtr<ID2D1SolidColorBrush> brush;
	unsigned int brushColor = 0;
	this->_deviceResources->_d2d1RenderTarget->CreateSolidColorBrush(D2D1::ColorF(brushColor), &brush);

	for (const auto& xwaBracket : g_xwa_bracket)
	{
		unsigned short si = ((unsigned short*)0x08D9420)[xwaBracket.colorIndex];
		unsigned int esi;

		if (((bool(*)())0x0050DC50)() != 0)
		{
			unsigned short eax = si & 0x001F;
			unsigned short ecx = si & 0x7C00;
			unsigned short edx = si & 0x03E0;

			esi = (eax << 3) | (edx << 6) | (ecx << 9);
		}
		else
		{
			unsigned short eax = si & 0x001F;
			unsigned short edx = si & 0xF800;
			unsigned short ecx = si & 0x07E0;

			esi = (eax << 3) | (ecx << 5) | (edx << 8);
		}

		if (esi != brushColor)
		{
			brushColor = esi;
			this->_deviceResources->_d2d1RenderTarget->CreateSolidColorBrush(D2D1::ColorF(brushColor), &brush);
		}

		float posX = left + (float)xwaBracket.positionX * scaleX;
		float posY = top + (float)xwaBracket.positionY * scaleY;
		float posW = (float)xwaBracket.width * scaleX;
		float posH = (float)xwaBracket.height * scaleY;
		float posSide = 0.125;

		float strokeWidth = 2.0f * std::min(scaleX, scaleY);

		bool fill = xwaBracket.width <= 4 || xwaBracket.height <= 4;

		if (fill)
		{
			this->_deviceResources->_d2d1RenderTarget->FillRectangle(D2D1::RectF(posX, posY, posX + posW, posY + posH), brush);
		}
		else
		{
			// top left
			this->_deviceResources->_d2d1RenderTarget->DrawLine(D2D1::Point2F(posX, posY), D2D1::Point2F(posX + posW * posSide, posY), brush, strokeWidth);
			this->_deviceResources->_d2d1RenderTarget->DrawLine(D2D1::Point2F(posX, posY), D2D1::Point2F(posX, posY + posH * posSide), brush, strokeWidth);

			// top right
			this->_deviceResources->_d2d1RenderTarget->DrawLine(D2D1::Point2F(posX + posW - posW * posSide, posY), D2D1::Point2F(posX + posW, posY), brush, strokeWidth);
			this->_deviceResources->_d2d1RenderTarget->DrawLine(D2D1::Point2F(posX + posW, posY), D2D1::Point2F(posX + posW, posY + posH * posSide), brush, strokeWidth);

			// bottom left
			this->_deviceResources->_d2d1RenderTarget->DrawLine(D2D1::Point2F(posX, posY + posH - posH * posSide), D2D1::Point2F(posX, posY + posH), brush, strokeWidth);
			this->_deviceResources->_d2d1RenderTarget->DrawLine(D2D1::Point2F(posX, posY + posH), D2D1::Point2F(posX + posW * posSide, posY + posH), brush, strokeWidth);

			// bottom right
			this->_deviceResources->_d2d1RenderTarget->DrawLine(D2D1::Point2F(posX + posW - posW * posSide, posY + posH), D2D1::Point2F(posX + posW, posY + posH), brush, strokeWidth);
			this->_deviceResources->_d2d1RenderTarget->DrawLine(D2D1::Point2F(posX + posW, posY + posH - posH * posSide), D2D1::Point2F(posX + posW, posY + posH), brush, strokeWidth);
		}
	}

	this->_deviceResources->_d2d1RenderTarget->EndDraw();
	this->_deviceResources->_d2d1RenderTarget->RestoreDrawingState(this->_deviceResources->_d2d1DrawingStateBlock);

	g_xwa_bracket.clear();
}
