// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include <emmintrin.h>
#include "DeviceResources.h"
#include "Direct3DDevice.h"
#include "Direct3DExecuteBuffer.h"
#include "BackbufferSurface.h"

Direct3DExecuteBuffer::Direct3DExecuteBuffer(DeviceResources* deviceResources, DWORD bufferSize, Direct3DDevice* d3dDevice)
{
	this->_refCount = 1;
	this->_deviceResources = deviceResources;
	this->_d3dDevice = d3dDevice;

	this->_bufferSize = bufferSize;
	this->_buffer = new char[bufferSize];
	this->_executeData = {};
}

Direct3DExecuteBuffer::~Direct3DExecuteBuffer()
{
	delete[] this->_buffer;
}

HRESULT Direct3DExecuteBuffer::QueryInterface(
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

ULONG Direct3DExecuteBuffer::AddRef()
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

ULONG Direct3DExecuteBuffer::Release()
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

HRESULT Direct3DExecuteBuffer::Initialize(
	LPDIRECT3DDEVICE lpDirect3DDevice,
	LPD3DEXECUTEBUFFERDESC lpDesc
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

HRESULT Direct3DExecuteBuffer::Lock(
	LPD3DEXECUTEBUFFERDESC lpDesc
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpDesc == nullptr)
	{
		return DDERR_INVALIDPARAMS;
	}

	if (this->_deviceResources->inScene && this->_deviceResources->inSceneBackbufferLocked)
	{
		if (this->_deviceResources->_displayBpp == 4)
		{
			int length = this->_deviceResources->_displayWidth * this->_deviceResources->_displayHeight;

			unsigned short* buffer16 = (unsigned short*)this->_deviceResources->_backbufferSurface->_buffer;
			unsigned int* buffer32 = (unsigned int*)this->_deviceResources->_backbufferSurface->_buffer;

			for (int i = length - 1; i >= 0; i--)
			{
				unsigned short color16 = buffer16[i * 2];

				if (color16 == 0)
				{
					buffer32[i] = 0xff000000;
				}
				else
				{
					buffer32[i] = convertColorB5G6R5toB8G8R8X8(color16);
				}
			}
			this->_deviceResources->RenderMain(this->_deviceResources->_backbufferSurface->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, this->_deviceResources->_displayBpp, RENDERMAIN_NO_COLORKEY);
		}
		else
		{
			// Need to replace transparent-black parts with colorkey in-place.
			// This fixes issues with the screen going all-black if we for some
			// reason render this buffer a second time.
			int length = this->_deviceResources->_displayWidth * this->_deviceResources->_displayHeight;

			unsigned short* buffer16 = (unsigned short*)this->_deviceResources->_backbufferSurface->_buffer;

			int i;
			// The loop condition shouldn't need to be so obfuscated,
			// but MSVC is stupid and generates slow loop conditions otherwise
			for (i = 16; i <= length; i += 16)
			{
				__m128i color16 = _mm_loadu_si128((const __m128i *)(buffer16 + i - 16));
				__m128i color16_2 = _mm_loadu_si128((const __m128i *)(buffer16 + i - 8));
				__m128i transparent = _mm_cmpeq_epi16(color16, _mm_set1_epi16(0));
				color16 = _mm_andnot_si128(transparent, color16);
				transparent = _mm_and_si128(transparent, _mm_set1_epi16(0x2000));
				color16 = _mm_or_si128(color16, transparent);
				_mm_storeu_si128((__m128i *)(buffer16 + i - 16), color16);

				transparent = _mm_cmpeq_epi16(color16_2, _mm_set1_epi16(0));
				color16_2 = _mm_andnot_si128(transparent, color16_2);
				transparent = _mm_and_si128(transparent, _mm_set1_epi16(0x2000));
				color16_2 = _mm_or_si128(color16_2, transparent);
				_mm_storeu_si128((__m128i *)(buffer16 + i - 8), color16_2);
			}
			i -= 16;
			for (; i < length; i++)
			{
				unsigned short color16 = buffer16[i];

				if (color16 == 0)
				{
					buffer16[i] = 0x2000;
				}
			}
			this->_deviceResources->RenderMain(this->_deviceResources->_backbufferSurface->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, this->_deviceResources->_displayBpp, RENDERMAIN_COLORKEY_20);
		}
		this->_deviceResources->inSceneBackbufferLocked = false;
	}

	//memset(this->_buffer, 0, this->_bufferSize);

	lpDesc->dwSize = sizeof(D3DEXECUTEBUFFERDESC);
	lpDesc->dwFlags = D3DDEB_BUFSIZE | D3DDEB_LPDATA;
	lpDesc->dwCaps = 0;
	lpDesc->dwBufferSize = this->_bufferSize;
	lpDesc->lpData = this->_buffer;

	if (g_config.D3dHookExists)
	{
		D3D11_MAPPED_SUBRESOURCE vertexMap;
		this->_deviceResources->_d3dDeviceContext->Map(this->_d3dDevice->_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &vertexMap);

		D3D11_MAPPED_SUBRESOURCE indexMap;
		this->_deviceResources->_d3dDeviceContext->Map(this->_d3dDevice->_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &indexMap);

		lpDesc->dwCaps = (DWORD)vertexMap.pData;
		lpDesc->dwFlags = (DWORD)indexMap.pData;
	}

	return D3D_OK;
}

HRESULT Direct3DExecuteBuffer::Unlock()
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (g_config.D3dHookExists)
	{
		this->_deviceResources->_d3dDeviceContext->Unmap(this->_d3dDevice->_vertexBuffer, 0);
		this->_deviceResources->_d3dDeviceContext->Unmap(this->_d3dDevice->_indexBuffer, 0);
	}

	return D3D_OK;
}

HRESULT Direct3DExecuteBuffer::SetExecuteData(
	LPD3DEXECUTEDATA lpData
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpData == nullptr)
	{
		return DDERR_INVALIDPARAMS;
	}

	this->_executeData = *lpData;

	return D3D_OK;
}

HRESULT Direct3DExecuteBuffer::GetExecuteData(
	LPD3DEXECUTEDATA lpData
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

HRESULT Direct3DExecuteBuffer::Validate(
	LPDWORD lpdwOffset,
	LPD3DVALIDATECALLBACK lpFunc,
	LPVOID lpUserArg,
	DWORD dwReserved
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

HRESULT Direct3DExecuteBuffer::Optimize(DWORD dwFlags)
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
