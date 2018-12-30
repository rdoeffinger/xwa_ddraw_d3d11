// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#pragma once

class Direct3DTexture;
struct ID3D11DeviceContext;
class RenderStates;

class Direct3DDevice : IDirect3DDevice
{
public:
	Direct3DDevice(DeviceResources* deviceResources);

	virtual ~Direct3DDevice();

	/*** IUnknown methods ***/

	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);

	STDMETHOD_(ULONG, AddRef)(THIS);

	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DDevice methods ***/

	STDMETHOD(Initialize)(THIS_ LPDIRECT3D, LPGUID, LPD3DDEVICEDESC);

	STDMETHOD(GetCaps)(THIS_ LPD3DDEVICEDESC, LPD3DDEVICEDESC);

	STDMETHOD(SwapTextureHandles)(THIS_ LPDIRECT3DTEXTURE, LPDIRECT3DTEXTURE);

	STDMETHOD(CreateExecuteBuffer)(THIS_ LPD3DEXECUTEBUFFERDESC, LPDIRECT3DEXECUTEBUFFER*, IUnknown*);

	STDMETHOD(GetStats)(THIS_ LPD3DSTATS);

	STDMETHOD(Execute)(THIS_ LPDIRECT3DEXECUTEBUFFER, LPDIRECT3DVIEWPORT, DWORD);

	STDMETHOD(AddViewport)(THIS_ LPDIRECT3DVIEWPORT);

	STDMETHOD(DeleteViewport)(THIS_ LPDIRECT3DVIEWPORT);

	STDMETHOD(NextViewport)(THIS_ LPDIRECT3DVIEWPORT, LPDIRECT3DVIEWPORT*, DWORD);

	STDMETHOD(Pick)(THIS_ LPDIRECT3DEXECUTEBUFFER, LPDIRECT3DVIEWPORT, DWORD, LPD3DRECT);

	STDMETHOD(GetPickRecords)(THIS_ LPDWORD, LPD3DPICKRECORD);

	STDMETHOD(EnumTextureFormats)(THIS_ LPD3DENUMTEXTUREFORMATSCALLBACK, LPVOID);

	STDMETHOD(CreateMatrix)(THIS_ LPD3DMATRIXHANDLE);

	STDMETHOD(SetMatrix)(THIS_ D3DMATRIXHANDLE, const LPD3DMATRIX);

	STDMETHOD(GetMatrix)(THIS_ D3DMATRIXHANDLE, LPD3DMATRIX);

	STDMETHOD(DeleteMatrix)(THIS_ D3DMATRIXHANDLE);

	STDMETHOD(BeginScene)(THIS);

	STDMETHOD(EndScene)(THIS);

	STDMETHOD(GetDirect3D)(THIS_ LPDIRECT3D*);

	ULONG _refCount;

	DeviceResources* _deviceResources;

	RenderStates* _renderStates;

	DWORD _maxExecuteBufferSize;
	ComPtr<ID3D11Buffer> _vertexBuffer;
	ComPtr<ID3D11Buffer> _indexBuffer;

private:
	void UpdatePixelShader(ID3D11DeviceContext *context, Direct3DTexture *texture);
};
