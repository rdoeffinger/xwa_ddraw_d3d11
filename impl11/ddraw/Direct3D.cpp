// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "DeviceResources.h"
#include "Direct3D.h"
#include "Direct3DMaterial.h"
#include "Direct3DViewport.h"

Direct3D::Direct3D(DeviceResources* deviceResources)
{
	g_config.runAutopatch();
	this->_refCount = 1;
	this->_deviceResources = deviceResources;
}

Direct3D::~Direct3D()
{
}

HRESULT Direct3D::QueryInterface(
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

ULONG Direct3D::AddRef()
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

ULONG Direct3D::Release()
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

HRESULT Direct3D::Initialize(REFCLSID riid)
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

HRESULT Direct3D::EnumDevices(
	LPD3DENUMDEVICESCALLBACK lpEnumDevicesCallback,
	LPVOID lpUserArg
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpEnumDevicesCallback == nullptr)
	{
		return DDERR_INVALIDPARAMS;
	}

	ComPtr<IDXGIDevice> dxgiDevice;
	if (FAILED(this->_deviceResources->_d3dDevice.As(&dxgiDevice)))
		return DDERR_INVALIDOBJECT;

	ComPtr<IDXGIAdapter> dxgiAdapter;
	if (FAILED(dxgiDevice->GetAdapter(&dxgiAdapter)))
		return DDERR_INVALIDOBJECT;

	DXGI_ADAPTER_DESC adapterDesc;
	if (FAILED(dxgiAdapter->GetDesc(&adapterDesc)))
		return DDERR_INVALIDOBJECT;

	ComPtr<IDXGIOutput> dxgiOutput;
	if (FAILED(dxgiAdapter->EnumOutputs(0, &dxgiOutput)))
		return DDERR_INVALIDOBJECT;

	DXGI_OUTPUT_DESC outputDesc;
	if (FAILED(dxgiOutput->GetDesc(&outputDesc)))
		return DDERR_INVALIDOBJECT;

	D3DDEVICEDESC desc{};
	desc.dwSize = sizeof(D3DDEVICEDESC);

	desc.dwFlags =
		D3DDD_COLORMODEL |
		D3DDD_TRICAPS |
		D3DDD_DEVICERENDERBITDEPTH |
		D3DDD_DEVICEZBUFFERBITDEPTH |
		D3DDD_MAXBUFFERSIZE |
		D3DDD_MAXVERTEXCOUNT;

	desc.dcmColorModel = D3DCOLOR_RGB;

	desc.dpcTriCaps.dwSize = sizeof(D3DPRIMCAPS);

	desc.dpcTriCaps.dwMiscCaps =
		D3DPMISCCAPS_CULLNONE |
		D3DPMISCCAPS_CULLCW |
		D3DPMISCCAPS_CULLCCW;

	desc.dpcTriCaps.dwRasterCaps = 0;

	desc.dpcTriCaps.dwZCmpCaps =
		D3DPCMPCAPS_NEVER |
		D3DPCMPCAPS_LESS |
		D3DPCMPCAPS_EQUAL |
		D3DPCMPCAPS_LESSEQUAL |
		D3DPCMPCAPS_GREATER |
		D3DPCMPCAPS_NOTEQUAL |
		D3DPCMPCAPS_GREATEREQUAL |
		D3DPCMPCAPS_ALWAYS;

	desc.dpcTriCaps.dwSrcBlendCaps = 0;

	desc.dpcTriCaps.dwDestBlendCaps = 0;

	desc.dpcTriCaps.dwAlphaCmpCaps =
		D3DPCMPCAPS_NEVER |
		D3DPCMPCAPS_LESS |
		D3DPCMPCAPS_EQUAL |
		D3DPCMPCAPS_LESSEQUAL |
		D3DPCMPCAPS_GREATER |
		D3DPCMPCAPS_NOTEQUAL |
		D3DPCMPCAPS_GREATEREQUAL |
		D3DPCMPCAPS_ALWAYS;

	desc.dpcTriCaps.dwShadeCaps =
		D3DPSHADECAPS_COLORFLATRGB |
		D3DPSHADECAPS_COLORGOURAUDRGB |
		D3DPSHADECAPS_COLORPHONGRGB |
		D3DPSHADECAPS_SPECULARFLATRGB |
		D3DPSHADECAPS_SPECULARGOURAUDRGB |
		D3DPSHADECAPS_SPECULARPHONGRGB |
		D3DPSHADECAPS_ALPHAFLATBLEND |
		D3DPSHADECAPS_ALPHAFLATSTIPPLED |
		D3DPSHADECAPS_ALPHAGOURAUDBLEND |
		D3DPSHADECAPS_ALPHAGOURAUDSTIPPLED |
		D3DPSHADECAPS_ALPHAPHONGBLEND |
		D3DPSHADECAPS_ALPHAPHONGSTIPPLED;

	desc.dpcTriCaps.dwTextureCaps =
		D3DPTEXTURECAPS_PERSPECTIVE |
		D3DPTEXTURECAPS_ALPHA;

	desc.dpcTriCaps.dwTextureFilterCaps =
		D3DPTFILTERCAPS_NEAREST |
		D3DPTFILTERCAPS_LINEAR |
		D3DPTFILTERCAPS_MIPNEAREST |
		D3DPTFILTERCAPS_MIPLINEAR |
		D3DPTFILTERCAPS_LINEARMIPNEAREST |
		D3DPTFILTERCAPS_LINEARMIPLINEAR;

	desc.dpcTriCaps.dwTextureBlendCaps =
		D3DPBLENDCAPS_ZERO |
		D3DPBLENDCAPS_ONE |
		D3DPBLENDCAPS_SRCCOLOR |
		D3DPBLENDCAPS_INVSRCCOLOR |
		D3DPBLENDCAPS_SRCALPHA |
		D3DPBLENDCAPS_INVSRCALPHA |
		D3DPBLENDCAPS_DESTALPHA |
		D3DPBLENDCAPS_INVDESTALPHA |
		D3DPBLENDCAPS_DESTCOLOR |
		D3DPBLENDCAPS_INVDESTCOLOR |
		D3DPBLENDCAPS_SRCALPHASAT |
		D3DPBLENDCAPS_BOTHSRCALPHA |
		D3DPBLENDCAPS_BOTHINVSRCALPHA;

	desc.dpcTriCaps.dwTextureAddressCaps =
		D3DPTADDRESSCAPS_WRAP |
		D3DPTADDRESSCAPS_MIRROR |
		D3DPTADDRESSCAPS_CLAMP |
		D3DPTADDRESSCAPS_BORDER |
		D3DPTADDRESSCAPS_INDEPENDENTUV;

	desc.dpcTriCaps.dwStippleWidth = 0;
	desc.dpcTriCaps.dwStippleHeight = 0;

	desc.dwDeviceRenderBitDepth = DDBD_32;
	desc.dwDeviceZBufferBitDepth = DDBD_32;

	desc.dwMaxBufferSize = 0;
	desc.dwMaxVertexCount = 0xffff;

	GUID guid = IID_IDirect3DHALDevice;
	std::string adapterStr = wchar_tostring(adapterDesc.Description);
	std::string outputStr = wchar_tostring(outputDesc.DeviceName);

	lpEnumDevicesCallback(&guid, (LPSTR)adapterStr.c_str(), (LPSTR)outputStr.c_str(), &desc, &desc, lpUserArg);

	return D3D_OK;
}

HRESULT Direct3D::CreateLight(
	LPDIRECT3DLIGHT* lplpDirect3DLight,
	IUnknown* pUnkOuter
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

HRESULT Direct3D::CreateMaterial(
	LPDIRECT3DMATERIAL* lplpDirect3DMaterial,
	IUnknown* pUnkOuter
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lplpDirect3DMaterial == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	*lplpDirect3DMaterial = new Direct3DMaterial();

	return DD_OK;
}

HRESULT Direct3D::CreateViewport(
	LPDIRECT3DVIEWPORT* lplpD3DViewport,
	IUnknown* pUnkOuter
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lplpD3DViewport == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	*lplpD3DViewport = new Direct3DViewport(this->_deviceResources);

#if LOGGER
	str.str("");
	str << "\t" << *lplpD3DViewport;
	LogText(str.str());
#endif

	return D3D_OK;
}

HRESULT Direct3D::FindDevice(
	LPD3DFINDDEVICESEARCH lpD3DFDS,
	LPD3DFINDDEVICERESULT lpD3DFDR
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
