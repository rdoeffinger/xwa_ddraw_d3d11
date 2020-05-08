// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "DeviceResources.h"
#include "DirectDraw.h"

#include "joystick.h"

#pragma comment(lib, "dxguid")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

class LibraryWrapper
{
public:
	LibraryWrapper(LPCSTR lpLibFileName)
	{
		char path[MAX_PATH];
		GetSystemDirectoryA(path, MAX_PATH);
		strcat_s(path, "\\");
		strcat_s(path, lpLibFileName);

		this->_module = LoadLibraryA(path);
	}

	~LibraryWrapper()
	{
		if (this->_module)
		{
			FreeLibrary(this->_module);
		}
	}

	HMODULE _module;
};

static LibraryWrapper ddraw("ddraw.dll");

#ifdef __MINGW32__
#define ASMJMP(name) __asm("jmp %0\n" :: "m"(name))
#else
#define ASMJMP(name) __asm { jmp name }
#endif

// Note: Do not put variable into function, as that would allow
// it to be initialized at first call instead of startup.
// We want the function to only consist of the jmp.
// MSVC at least currently together with "naked" does this
// automatically, but better not to rely on it.
#define WRAP_FUNCTION(name) \
static void (*ddraw_proc_##name)() = (void(*)())GetProcAddress(ddraw._module, #name); \
extern "C" __declspec(naked) void name() \
{ \
    ASMJMP(ddraw_proc_##name); \
}

WRAP_FUNCTION(AcquireDDThreadLock)
WRAP_FUNCTION(CompleteCreateSysmemSurface)
WRAP_FUNCTION(D3DParseUnknownCommand)
WRAP_FUNCTION(DDGetAttachedSurfaceLcl)
WRAP_FUNCTION(DDInternalLock)
WRAP_FUNCTION(DDInternalUnlock)
WRAP_FUNCTION(DSoundHelp)

extern "C" HRESULT WINAPI DirectDrawCreate(
	_In_   GUID FAR *lpGUID,
	_Out_  LPDIRECTDRAW FAR *lplpDD,
	_In_   IUnknown FAR *pUnkOuter
	)
{
	//static decltype(DirectDrawCreate)* ddraw_proc = (decltype(DirectDrawCreate)*)GetProcAddress(ddraw._module, "DirectDrawCreate");
	//return ddraw_proc(lpGUID, lplpDD, pUnkOuter);

#if LOGGER
	std::ostringstream str;
	str << __FUNCTION__;
	str << " " << (lpGUID == nullptr ? "NULL" : tostr_IID(*lpGUID));
	LogText(str.str());
#endif

	if (lplpDD == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	DeviceResources* deviceResources = new DeviceResources();

	if (FAILED(deviceResources->Initialize()))
	{
		delete deviceResources;

#if LOGGER
		str.str("\tDDERR_NODIRECTDRAWHW");
		LogText(str.str());
#endif

		return DDERR_NODIRECTDRAWHW;
	}

	*lplpDD = new DirectDraw(deviceResources);

#if LOGGER
	str.str("");
	str << "\tDD_OK\t" << *lplpDD;
	LogText(str.str());
#endif

	return DD_OK;
}

extern "C" HRESULT WINAPI DirectDrawCreateClipper(
	_In_   DWORD dwFlags,
	_Out_  LPDIRECTDRAWCLIPPER FAR *lplpDDClipper,
	_In_   IUnknown FAR *pUnkOuter
	)
{
	static decltype(DirectDrawCreateClipper)* ddraw_proc = (decltype(DirectDrawCreateClipper)*)GetProcAddress(ddraw._module, "DirectDrawCreateClipper");

#if LOGGER
	std::ostringstream str;
	str << __FUNCTION__;
	LogText(str.str());
#endif

	return ddraw_proc(dwFlags, lplpDDClipper, pUnkOuter);
}

extern "C" HRESULT WINAPI DirectDrawCreateEx(
	_In_   GUID FAR *lpGUID,
	_Out_  LPVOID *lplpDD,
	_In_   REFIID iid,
	_In_   IUnknown FAR *pUnkOuter
	)
{
	static decltype(DirectDrawCreateEx)* ddraw_proc = (decltype(DirectDrawCreateEx)*)GetProcAddress(ddraw._module, "DirectDrawCreateEx");

#if LOGGER
	std::ostringstream str;
	str << __FUNCTION__;
	str << " " << (lpGUID == nullptr ? "NULL" : tostr_IID(*lpGUID));
	LogText(str.str());
#endif

	return ddraw_proc(lpGUID, lplpDD, iid, pUnkOuter);
}

extern "C" HRESULT WINAPI DirectDrawEnumerateA(
	_In_  LPDDENUMCALLBACKA lpCallback,
	_In_  LPVOID lpContext
	)
{
	static decltype(DirectDrawEnumerateA)* ddraw_proc = (decltype(DirectDrawEnumerateA)*)GetProcAddress(ddraw._module, "DirectDrawEnumerateA");

#if LOGGER
	std::ostringstream str;
	str << __FUNCTION__;
	LogText(str.str());
#endif

	return ddraw_proc(lpCallback, lpContext);
}

extern "C" HRESULT WINAPI DirectDrawEnumerateExA(
	_In_  LPDDENUMCALLBACKEXA lpCallback,
	_In_  LPVOID lpContext,
	_In_  DWORD dwFlags
	)
{
	//static decltype(DirectDrawEnumerateExA)* ddraw_proc = (decltype(DirectDrawEnumerateExA)*)GetProcAddress(ddraw._module, "DirectDrawEnumerateExA");
	//return ddraw_proc(lpCallback, lpContext, dwFlags);

#if LOGGER
	std::ostringstream str;
	str << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpCallback == nullptr)
	{
		return DDERR_INVALIDPARAMS;
	}

	ComPtr<IDXGIFactory> factory;

	if (FAILED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
	{
		return S_OK;
	}

	ComPtr<IDXGIAdapter> adapter;

	for (int i = 0; factory->EnumAdapters(i, &adapter) == S_OK; i++)
	{
		DXGI_ADAPTER_DESC adapterDesc;

		if (FAILED(adapter->GetDesc(&adapterDesc)))
		{
			continue;
		}

		std::string adapterStr = wchar_tostring(adapterDesc.Description);

		ComPtr<IDXGIOutput> output;

		for (int j = 0; adapter->EnumOutputs(j, &output) == S_OK; j++)
		{
			DXGI_OUTPUT_DESC outputDesc;

			if (FAILED(output->GetDesc(&outputDesc)))
			{
				continue;
			}

			std::string outputStr = wchar_tostring(outputDesc.DeviceName);

#if LOGGER
			str.str("");
			str << "\t" << outputStr << "\t" << adapterStr;
			LogText(str.str());
#endif

			if (lpCallback(nullptr, (LPSTR)adapterStr.c_str(), (LPSTR)outputStr.c_str(), lpContext, nullptr) != DDENUMRET_OK)
			{
				return DD_OK;
			}
		}
	}

	return DD_OK;
}

extern "C" HRESULT WINAPI DirectDrawEnumerateExW(
	_In_  LPDDENUMCALLBACKEXW lpCallback,
	_In_  LPVOID lpContext,
	_In_  DWORD dwFlags
	)
{
	static decltype(DirectDrawEnumerateExW)* ddraw_proc = (decltype(DirectDrawEnumerateExW)*)GetProcAddress(ddraw._module, "DirectDrawEnumerateExW");

#if LOGGER
	std::ostringstream str;
	str << __FUNCTION__;
	LogText(str.str());
#endif

	return ddraw_proc(lpCallback, lpContext, dwFlags);
}

extern "C" HRESULT WINAPI DirectDrawEnumerateW(
	_In_  LPDDENUMCALLBACKW lpCallback,
	_In_  LPVOID lpContext
	)
{
	static decltype(DirectDrawEnumerateW)* ddraw_proc = (decltype(DirectDrawEnumerateW)*)GetProcAddress(ddraw._module, "DirectDrawEnumerateW");

#if LOGGER
	std::ostringstream str;
	str << __FUNCTION__;
	LogText(str.str());
#endif

	return ddraw_proc(lpCallback, lpContext);
}

WRAP_FUNCTION(GetDDSurfaceLocal)
WRAP_FUNCTION(GetOLEThunkData)
WRAP_FUNCTION(GetSurfaceFromDC)
WRAP_FUNCTION(RegisterSpecialCase)
WRAP_FUNCTION(ReleaseDDThreadLock)
WRAP_FUNCTION(SetAppCompatData)