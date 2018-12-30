// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include <emmintrin.h>
#include "DeviceResources.h"
#include "Direct3DDevice.h"
#include "Direct3DExecuteBuffer.h"
#include "Direct3DTexture.h"
#include "BackbufferSurface.h"
#include "ExecuteBufferDumper.h"
#include "TextureSurface.h"

class RenderStates
{
public:
	RenderStates(DeviceResources* deviceResources)
	{
		this->_deviceResources = deviceResources;

		this->TextureAddress = D3DTADDRESS_WRAP;
		this->TextureMag = D3DFILTER_LINEARMIPLINEAR;
		this->TextureMin = D3DFILTER_LINEARMIPLINEAR;

		this->AlphaBlendEnabled = FALSE;
		this->TextureMapBlend = D3DTBLEND_MODULATE;
		this->SrcBlend = D3DBLEND_ONE;
		this->DestBlend = D3DBLEND_ZERO;

		this->ZEnabled = TRUE;
		this->ZWriteEnabled = TRUE;
		this->ZFunc = D3DCMP_GREATER;

		this->AlphaTestEnabled = false;
		this->AlphaFunc = D3DCMP_ALWAYS;
		this->AlphaRef = 0;
	}

	static D3D11_TEXTURE_ADDRESS_MODE TextureAddressMode(D3DTEXTUREADDRESS address)
	{
		switch (address)
		{
		case D3DTADDRESS_WRAP:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case D3DTADDRESS_MIRROR:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		case D3DTADDRESS_CLAMP:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		}

		return D3D11_TEXTURE_ADDRESS_WRAP;
	}

	static D3D11_BLEND Blend(D3DBLEND blend)
	{
		switch (blend)
		{
		case D3DBLEND_ZERO:
			return D3D11_BLEND_ZERO;
		case D3DBLEND_ONE:
			return D3D11_BLEND_ONE;
		case D3DBLEND_SRCCOLOR:
			return D3D11_BLEND_SRC_COLOR;
		case D3DBLEND_INVSRCCOLOR:
			return D3D11_BLEND_INV_SRC_COLOR;
		case D3DBLEND_SRCALPHA:
			return D3D11_BLEND_SRC_ALPHA;
		case D3DBLEND_INVSRCALPHA:
			return D3D11_BLEND_INV_SRC_ALPHA;
		case D3DBLEND_DESTALPHA:
			return D3D11_BLEND_DEST_ALPHA;
		case D3DBLEND_INVDESTALPHA:
			return D3D11_BLEND_INV_DEST_ALPHA;
		case D3DBLEND_DESTCOLOR:
			return D3D11_BLEND_DEST_COLOR;
		case D3DBLEND_INVDESTCOLOR:
			return D3D11_BLEND_INV_DEST_COLOR;
		case D3DBLEND_SRCALPHASAT:
			return D3D11_BLEND_SRC_ALPHA_SAT;
		case D3DBLEND_BOTHSRCALPHA:
			return D3D11_BLEND_SRC1_ALPHA;
		case D3DBLEND_BOTHINVSRCALPHA:
			return D3D11_BLEND_INV_SRC1_ALPHA;
		}

		return D3D11_BLEND_ZERO;
	}

	static D3D11_COMPARISON_FUNC ComparisonFunc(D3DCMPFUNC func)
	{
		switch (func)
		{
		case D3DCMP_NEVER:
			return D3D11_COMPARISON_NEVER;
		case D3DCMP_LESS:
			return D3D11_COMPARISON_LESS;
		case D3DCMP_EQUAL:
			return D3D11_COMPARISON_EQUAL;
		case D3DCMP_LESSEQUAL:
			return D3D11_COMPARISON_LESS_EQUAL;
		case D3DCMP_GREATER:
			return D3D11_COMPARISON_GREATER;
		case D3DCMP_NOTEQUAL:
			return D3D11_COMPARISON_NOT_EQUAL;
		case D3DCMP_GREATEREQUAL:
			return D3D11_COMPARISON_GREATER_EQUAL;
		case D3DCMP_ALWAYS:
			return D3D11_COMPARISON_ALWAYS;
		}

		return D3D11_COMPARISON_ALWAYS;
	}

	D3D11_SAMPLER_DESC GetSamplerDesc()
	{
		D3D11_SAMPLER_DESC desc;
		desc.Filter = this->_deviceResources->_useAnisotropy ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		// Support disabling bilinear filtering in settings for
		// e.g. X-Wing vs. TIE
		if (this->TextureMag == D3DFILTER_NEAREST && this->TextureMin == D3DFILTER_NEAREST) {
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		}
		desc.MaxAnisotropy = this->_deviceResources->_useAnisotropy ? this->_deviceResources->GetMaxAnisotropy() : 1;
		desc.AddressU = TextureAddressMode(this->TextureAddress);
		desc.AddressV = TextureAddressMode(this->TextureAddress);
		desc.AddressW = TextureAddressMode(this->TextureAddress);
		desc.MipLODBias = 0.0f;
		desc.MinLOD = 0;
		desc.MaxLOD = FLT_MAX;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.BorderColor[0] = 0.0f;
		desc.BorderColor[1] = 0.0f;
		desc.BorderColor[2] = 0.0f;
		desc.BorderColor[3] = 0.0f;

		return desc;
	}

	D3D11_BLEND_DESC GetBlendDesc()
	{
		D3D11_BLEND_DESC desc{};

		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;
		desc.RenderTarget[0].BlendEnable = this->AlphaBlendEnabled;
		desc.RenderTarget[0].SrcBlend = Blend(this->SrcBlend);
		desc.RenderTarget[0].DestBlend = Blend(this->DestBlend);
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

		desc.RenderTarget[0].SrcBlendAlpha = this->TextureMapBlend == D3DTBLEND_MODULATEALPHA ? D3D11_BLEND_SRC_ALPHA : D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = this->TextureMapBlend == D3DTBLEND_MODULATEALPHA ? D3D11_BLEND_INV_SRC_ALPHA : D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		return desc;
	}

	D3D11_DEPTH_STENCIL_DESC GetDepthStencilDesc()
	{
		D3D11_DEPTH_STENCIL_DESC desc{};

		desc.DepthEnable = this->ZEnabled;
		desc.DepthWriteMask = this->ZWriteEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.DepthFunc = ComparisonFunc(this->ZFunc);
		desc.StencilEnable = FALSE;

		return desc;
	}

	inline void SetTextureAddress(D3DTEXTUREADDRESS textureAddress)
	{
		this->TextureAddress = textureAddress;
	}

	inline void SetTextureMag(D3DTEXTUREFILTER textureMag)
	{
		this->TextureMag = textureMag;
	}

	inline void SetTextureMin(D3DTEXTUREFILTER textureMin)
	{
		this->TextureMin = textureMin;
	}

	inline void SetAlphaBlendEnabled(BOOL alphaBlendEnabled)
	{
		this->AlphaBlendEnabled = alphaBlendEnabled;
	}

	inline void SetTextureMapBlend(D3DTEXTUREBLEND textureMapBlend)
	{
		this->TextureMapBlend = textureMapBlend;
	}

	inline void SetSrcBlend(D3DBLEND srcBlend)
	{
		this->SrcBlend = srcBlend;
	}

	inline void SetDestBlend(D3DBLEND destBlend)
	{
		this->DestBlend = destBlend;
	}

	inline void SetZEnabled(BOOL zEnabled)
	{
		this->ZEnabled = zEnabled;
	}

	inline void SetZWriteEnabled(BOOL zWriteEnabled)
	{
		this->ZWriteEnabled = zWriteEnabled;
	}

	inline void SetZFunc(D3DCMPFUNC zFunc)
	{
		this->ZFunc = zFunc;
	}

public:
	bool AlphaTestEnabled;
	D3DCMPFUNC AlphaFunc;
	DWORD AlphaRef;

private:
	DeviceResources* _deviceResources;

	D3DTEXTUREADDRESS TextureAddress;
	D3DTEXTUREFILTER TextureMag;
	D3DTEXTUREFILTER TextureMin;

	BOOL AlphaBlendEnabled;
	D3DTEXTUREBLEND TextureMapBlend;
	D3DBLEND SrcBlend;
	D3DBLEND DestBlend;

	BOOL ZEnabled;
	BOOL ZWriteEnabled;
	D3DCMPFUNC ZFunc;
};

Direct3DDevice::Direct3DDevice(DeviceResources* deviceResources)
{
	this->_refCount = 1;
	this->_deviceResources = deviceResources;

	this->_renderStates = new RenderStates(this->_deviceResources);

	this->_maxExecuteBufferSize = 0;
}

Direct3DDevice::~Direct3DDevice()
{
	delete this->_renderStates;
}

HRESULT Direct3DDevice::QueryInterface(
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

ULONG Direct3DDevice::AddRef()
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

ULONG Direct3DDevice::Release()
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

HRESULT Direct3DDevice::Initialize(
	LPDIRECT3D lpd3d,
	LPGUID lpGUID,
	LPD3DDEVICEDESC lpd3ddvdesc
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

HRESULT Direct3DDevice::GetCaps(
	LPD3DDEVICEDESC lpD3DHWDevDesc,
	LPD3DDEVICEDESC lpD3DHELDevDesc
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

HRESULT Direct3DDevice::SwapTextureHandles(
	LPDIRECT3DTEXTURE lpD3DTex1,
	LPDIRECT3DTEXTURE lpD3DTex2
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

HRESULT Direct3DDevice::CreateExecuteBuffer(
	LPD3DEXECUTEBUFFERDESC lpDesc,
	LPDIRECT3DEXECUTEBUFFER *lplpDirect3DExecuteBuffer,
	IUnknown *pUnkOuter
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpDesc == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	if ((lpDesc->dwFlags & D3DDEB_BUFSIZE) == 0)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	if (lplpDirect3DExecuteBuffer == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	if (lpDesc->dwBufferSize > this->_maxExecuteBufferSize)
	{
		auto& device = this->_deviceResources->_d3dDevice;

		D3D11_BUFFER_DESC vertexBufferDesc;
		vertexBufferDesc.ByteWidth = lpDesc->dwBufferSize;
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		if (FAILED(device->CreateBuffer(&vertexBufferDesc, nullptr, &this->_vertexBuffer)))
			return DDERR_INVALIDOBJECT;

		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.ByteWidth = lpDesc->dwBufferSize;
		indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.StructureByteStride = 0;

		if (FAILED(device->CreateBuffer(&indexBufferDesc, nullptr, &this->_indexBuffer)))
			return DDERR_INVALIDOBJECT;

		this->_maxExecuteBufferSize = lpDesc->dwBufferSize;
	}

	*lplpDirect3DExecuteBuffer = new Direct3DExecuteBuffer(this->_deviceResources, lpDesc->dwBufferSize);

#if LOGGER
	str.str("");
	str << "\t" << *lplpDirect3DExecuteBuffer;
	LogText(str.str());
#endif

	return D3D_OK;
}

HRESULT Direct3DDevice::GetStats(
	LPD3DSTATS lpD3DStats
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

void Direct3DDevice::UpdatePixelShader(ID3D11DeviceContext *context, Direct3DTexture *texture)
{
	ID3D11PixelShader* pixelShader;

	if (texture == nullptr)
	{
		ID3D11ShaderResourceView* view = nullptr;
		context->PSSetShaderResources(0, 1, &view);

		pixelShader = this->_deviceResources->_pixelShaderSolid;
	}
	else if (!this->_renderStates->AlphaTestEnabled || this->_renderStates->AlphaFunc == D3DCMP_ALWAYS)
	{
		context->PSSetShaderResources(0, 1, texture->_textureView.GetAddressOf());

		pixelShader = this->_deviceResources->_pixelShaderTexture;
	}
	else
	{
		context->PSSetShaderResources(0, 1, texture->_textureView.GetAddressOf());

#if LOGGER
		if (this->_renderStates->AlphaFunc != D3DCMP_NOTEQUAL || this->_renderStates->AlphaRef != 0)
			LogText("Unsupported alpha test setting!");
#endif
		// NOTE/HACK: Using alpha interpolation for ATEST textures in TIE Fighter results in starfield shining
		// through the edges of light source textures. In BoP it causes distant frigates to be full of "holes" etc.
		bool alpha_single_bit = texture->_surface->_pixelFormat.dwRGBBitCount == 16 && texture->_surface->_pixelFormat.dwRGBAlphaBitMask == 0x8000;
		pixelShader = alpha_single_bit ? this->_deviceResources->_pixelShaderAtestTextureNoAlpha : this->_deviceResources->_pixelShaderAtestTexture;
	}

	this->_deviceResources->InitPixelShader(pixelShader);
}


HRESULT Direct3DDevice::Execute(
	LPDIRECT3DEXECUTEBUFFER lpDirect3DExecuteBuffer,
	LPDIRECT3DVIEWPORT lpDirect3DViewport,
	DWORD dwFlags
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpDirect3DExecuteBuffer == nullptr)
	{
#if LOGGER
		str.str("\tDDERR_INVALIDPARAMS");
		LogText(str.str());
#endif

		return DDERR_INVALIDPARAMS;
	}

	Direct3DExecuteBuffer* executeBuffer = (Direct3DExecuteBuffer*)lpDirect3DExecuteBuffer;

#if LOGGER
	DumpExecuteBuffer(executeBuffer);
#endif

	auto& device = this->_deviceResources->_d3dDevice;
	auto& context = this->_deviceResources->_d3dDeviceContext;

	HRESULT hr = S_OK;
	const char* step = "";

	this->_deviceResources->InitInputLayout(this->_deviceResources->_inputLayout);
	this->_deviceResources->InitVertexShader(this->_deviceResources->_vertexShader);
	this->_deviceResources->InitPixelShader(this->_deviceResources->_pixelShaderTexture);
	this->_deviceResources->InitTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	this->_deviceResources->InitRasterizerState(this->_deviceResources->_rasterizerState);

	if (SUCCEEDED(hr))
	{
		step = "ConstantBuffer";

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

		float scale;

		if (!g_config.XWAMode || this->_deviceResources->_frontbufferSurface == nullptr)
		{
			scale = 1.0f;
		}
		else
		{
			if (this->_deviceResources->_backbufferHeight * this->_deviceResources->_displayWidth <= this->_deviceResources->_backbufferWidth * this->_deviceResources->_displayHeight)
			{
				scale = (float)this->_deviceResources->_backbufferHeight / (float)this->_deviceResources->_displayHeight;
			}
			else
			{
				scale = (float)this->_deviceResources->_backbufferWidth / (float)this->_deviceResources->_displayWidth;
			}

			scale *= g_config.Concourse3DScale;
		}

		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = (float)left;
		viewport.TopLeftY = (float)top;
		viewport.Width = (float)w;
		viewport.Height = (float)h;
		viewport.MinDepth = D3D11_MIN_DEPTH;
		viewport.MaxDepth = D3D11_MAX_DEPTH;
		this->_deviceResources->InitViewport(&viewport);

		const float viewportScale[4] = { 2.0f / (float)this->_deviceResources->_displayWidth, -2.0f / (float)this->_deviceResources->_displayHeight, scale, 0 };
		this->_deviceResources->InitConstantBuffer(this->_deviceResources->_constantBuffer.GetAddressOf(), viewportScale);
	}

	if (SUCCEEDED(hr))
	{
		step = "VertexBuffer";

		D3D11_MAPPED_SUBRESOURCE map;
		hr = context->Map(this->_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);

		if (SUCCEEDED(hr))
		{
			size_t length = sizeof(D3DTLVERTEX) * executeBuffer->_executeData.dwVertexCount;
			memcpy(map.pData, executeBuffer->_buffer, length);
			//memset((char*)map.pData + length, 0, this->_maxExecuteBufferSize - length);

			context->Unmap(this->_vertexBuffer, 0);
		}

		if (SUCCEEDED(hr))
		{
			UINT stride = sizeof(D3DTLVERTEX);
			UINT offset = 0;

			this->_deviceResources->InitVertexBuffer(this->_vertexBuffer.GetAddressOf(), &stride, &offset);
		}
	}

	if (SUCCEEDED(hr))
	{
		step = "IndexBuffer";

		D3D11_MAPPED_SUBRESOURCE map;
		hr = context->Map(this->_indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);

		if (SUCCEEDED(hr))
		{
			char* pData = executeBuffer->_buffer + executeBuffer->_executeData.dwInstructionOffset;
			char* pDataEnd = pData + executeBuffer->_executeData.dwInstructionLength;

			WORD* indice = (WORD*)map.pData;

			while (pData < pDataEnd)
			{
				LPD3DINSTRUCTION instruction = (LPD3DINSTRUCTION)pData;
				pData += sizeof(D3DINSTRUCTION) + instruction->bSize * instruction->wCount;

				switch (instruction->bOpcode)
				{
				case D3DOP_TRIANGLE:
				{
					LPD3DTRIANGLE triangle = (LPD3DTRIANGLE)(instruction + 1);

					for (WORD i = 0; i < instruction->wCount; i++)
					{
						*indice = triangle->v1;
						indice++;

						*indice = triangle->v2;
						indice++;

						*indice = triangle->v3;
						indice++;

						triangle++;
					}
				}
				}
			}

			context->Unmap(this->_indexBuffer, 0);
		}

		if (SUCCEEDED(hr))
		{
			this->_deviceResources->InitIndexBuffer(this->_indexBuffer);
		}
	}

	if (SUCCEEDED(hr))
	{
		char* pData = executeBuffer->_buffer + executeBuffer->_executeData.dwInstructionOffset;
		char* pDataEnd = pData + executeBuffer->_executeData.dwInstructionLength;

		Direct3DTexture* currentTexture = nullptr;

		UINT currentIndexLocation = 0;

		while (pData < pDataEnd)
		{
			LPD3DINSTRUCTION instruction = (LPD3DINSTRUCTION)pData;
			pData += sizeof(D3DINSTRUCTION) + instruction->bSize * instruction->wCount;

			switch (instruction->bOpcode)
			{
			case D3DOP_STATERENDER:
			{
				LPD3DSTATE state = (LPD3DSTATE)(instruction + 1);

				for (WORD i = 0; i < instruction->wCount; i++)
				{
					switch (state->drstRenderStateType)
					{
					case D3DRENDERSTATE_TEXTUREHANDLE:
					{
						currentTexture = g_config.WireframeFillMode ? nullptr : (Direct3DTexture*)state->dwArg[0];
						UpdatePixelShader(context, currentTexture);
						break;
					}

					case D3DRENDERSTATE_TEXTUREADDRESS:
						this->_renderStates->SetTextureAddress((D3DTEXTUREADDRESS)state->dwArg[0]);
						break;

					case D3DRENDERSTATE_TEXTUREMAG:
						this->_renderStates->SetTextureMag((D3DTEXTUREFILTER)state->dwArg[0]);
						break;
					case D3DRENDERSTATE_TEXTUREMIN:
						this->_renderStates->SetTextureMin((D3DTEXTUREFILTER)state->dwArg[0]);
						break;

					case D3DRENDERSTATE_ALPHABLENDENABLE:
						this->_renderStates->SetAlphaBlendEnabled(state->dwArg[0]);
						break;
					case D3DRENDERSTATE_TEXTUREMAPBLEND:
						this->_renderStates->SetTextureMapBlend((D3DTEXTUREBLEND)state->dwArg[0]);
						break;
					case D3DRENDERSTATE_SRCBLEND:
						this->_renderStates->SetSrcBlend((D3DBLEND)state->dwArg[0]);
						break;
					case D3DRENDERSTATE_DESTBLEND:
						this->_renderStates->SetDestBlend((D3DBLEND)state->dwArg[0]);
						break;
					case D3DRENDERSTATE_ALPHATESTENABLE:
						this->_renderStates->AlphaTestEnabled = state->dwArg[0] != 0;
						UpdatePixelShader(context, currentTexture);
						break;
					case D3DRENDERSTATE_ALPHAFUNC:
						this->_renderStates->AlphaFunc = (D3DCMPFUNC)state->dwArg[0];
						UpdatePixelShader(context, currentTexture);
						break;
					case D3DRENDERSTATE_ALPHAREF:
						this->_renderStates->AlphaRef = state->dwArg[0];
						UpdatePixelShader(context, currentTexture);
						break;
					case D3DRENDERSTATE_ZENABLE:
						this->_renderStates->SetZEnabled(state->dwArg[0]);
						break;
					case D3DRENDERSTATE_ZWRITEENABLE:
						this->_renderStates->SetZWriteEnabled(state->dwArg[0]);
						break;
					case D3DRENDERSTATE_ZFUNC:
						this->_renderStates->SetZFunc((D3DCMPFUNC)state->dwArg[0]);
						break;
					}

					state++;
				}

				break;
			}

			case D3DOP_TRIANGLE:
			{
				step = "SamplerState";
				D3D11_SAMPLER_DESC samplerDesc = this->_renderStates->GetSamplerDesc();
				if (FAILED(hr = this->_deviceResources->InitSamplerState(nullptr, &samplerDesc)))
					break;

				step = "BlendState";
				D3D11_BLEND_DESC blendDesc = this->_renderStates->GetBlendDesc();
				if (FAILED(hr = this->_deviceResources->InitBlendState(nullptr, &blendDesc)))
					break;

				step = "DepthStencilState";
				D3D11_DEPTH_STENCIL_DESC depthDesc = this->_renderStates->GetDepthStencilDesc();
				if (FAILED(hr = this->_deviceResources->InitDepthStencilState(nullptr, &depthDesc)))
					break;

				step = "Draw";
				context->DrawIndexed(3 * instruction->wCount, currentIndexLocation, 0);

				currentIndexLocation += 3 * instruction->wCount;
				break;
			}
			}

			if (FAILED(hr))
				break;
		}
	}

	if (FAILED(hr))
	{
		static bool messageShown = false;

		if (!messageShown)
		{
			char text[512];
			strcpy_s(text, step);
			strcat_s(text, "\n");
			strcat_s(text, _com_error(hr).ErrorMessage());

			MessageBox(nullptr, text, __FUNCTION__, MB_ICONERROR);
		}

		messageShown = true;

#if LOGGER
		str.str("\tD3DERR_EXECUTE_FAILED");
		LogText(str.str());
#endif

		return D3DERR_EXECUTE_FAILED;
	}

#if LOGGER
	str.str("\tD3D_OK");
	LogText(str.str());
#endif

	return D3D_OK;
}

HRESULT Direct3DDevice::AddViewport(
	LPDIRECT3DVIEWPORT lpDirect3DViewport
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tD3D_OK");
	LogText(str.str());
#endif

	return D3D_OK;
}

HRESULT Direct3DDevice::DeleteViewport(
	LPDIRECT3DVIEWPORT lpDirect3DViewport
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

#if LOGGER
	str.str("\tD3D_OK");
	LogText(str.str());
#endif

	return D3D_OK;
}

HRESULT Direct3DDevice::NextViewport(
	LPDIRECT3DVIEWPORT lpDirect3DViewport,
	LPDIRECT3DVIEWPORT *lplpDirect3DViewport,
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

HRESULT Direct3DDevice::Pick(
	LPDIRECT3DEXECUTEBUFFER lpDirect3DExecuteBuffer,
	LPDIRECT3DVIEWPORT lpDirect3DViewport,
	DWORD dwFlags,
	LPD3DRECT lpRect
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

HRESULT Direct3DDevice::GetPickRecords(
	LPDWORD lpCount,
	LPD3DPICKRECORD lpD3DPickRec
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

HRESULT Direct3DDevice::EnumTextureFormats(
	LPD3DENUMTEXTUREFORMATSCALLBACK lpd3dEnumTextureProc,
	LPVOID lpArg
	)
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (lpd3dEnumTextureProc == nullptr)
	{
		return DDERR_INVALIDPARAMS;
	}

	DDSURFACEDESC sd{};
	sd.dwSize = sizeof(DDSURFACEDESC);
	sd.dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
	sd.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	sd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

	// 0555
	sd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	sd.ddpfPixelFormat.dwRGBBitCount = 16;
	sd.ddpfPixelFormat.dwRBitMask = 0x7C00;
	sd.ddpfPixelFormat.dwGBitMask = 0x03E0;
	sd.ddpfPixelFormat.dwBBitMask = 0x001F;
	sd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x0000;

	if (lpd3dEnumTextureProc(&sd, lpArg) != D3DENUMRET_OK)
		return D3D_OK;

	// 1555
	sd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
	sd.ddpfPixelFormat.dwRGBBitCount = 16;
	sd.ddpfPixelFormat.dwRBitMask = 0x7C00;
	sd.ddpfPixelFormat.dwGBitMask = 0x03E0;
	sd.ddpfPixelFormat.dwBBitMask = 0x001F;
	sd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x8000;

	if (lpd3dEnumTextureProc(&sd, lpArg) != D3DENUMRET_OK)
		return D3D_OK;

	// 4444
	sd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
	sd.ddpfPixelFormat.dwRGBBitCount = 16;
	sd.ddpfPixelFormat.dwRBitMask = 0x0F00;
	sd.ddpfPixelFormat.dwGBitMask = 0x00F0;
	sd.ddpfPixelFormat.dwBBitMask = 0x000F;
	sd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xF000;

	if (lpd3dEnumTextureProc(&sd, lpArg) != D3DENUMRET_OK)
		return D3D_OK;

	// 0565
	sd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	sd.ddpfPixelFormat.dwRGBBitCount = 16;
	sd.ddpfPixelFormat.dwRBitMask = 0xF800;
	sd.ddpfPixelFormat.dwGBitMask = 0x07E0;
	sd.ddpfPixelFormat.dwBBitMask = 0x001F;
	sd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x0000;

	if (lpd3dEnumTextureProc(&sd, lpArg) != D3DENUMRET_OK)
		return D3D_OK;

	// 0888
	sd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	sd.ddpfPixelFormat.dwRGBBitCount = 32;
	sd.ddpfPixelFormat.dwRBitMask = 0x0FF0000;
	sd.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
	sd.ddpfPixelFormat.dwBBitMask = 0x000000FF;
	sd.ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000;

	if (lpd3dEnumTextureProc(&sd, lpArg) != D3DENUMRET_OK)
		return D3D_OK;

	// 8888
	sd.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
	sd.ddpfPixelFormat.dwRGBBitCount = 32;
	sd.ddpfPixelFormat.dwRBitMask = 0x00FF0000;
	sd.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
	sd.ddpfPixelFormat.dwBBitMask = 0x000000FF;
	sd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF000000;

	if (lpd3dEnumTextureProc(&sd, lpArg) != D3DENUMRET_OK)
		return D3D_OK;

	return D3D_OK;
}

HRESULT Direct3DDevice::CreateMatrix(
	LPD3DMATRIXHANDLE lpD3DMatHandle
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

HRESULT Direct3DDevice::SetMatrix(
	D3DMATRIXHANDLE d3dMatHandle,
	LPD3DMATRIX lpD3DMatrix
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

HRESULT Direct3DDevice::GetMatrix(
	D3DMATRIXHANDLE d3dMatHandle,
	LPD3DMATRIX lpD3DMatrix
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

HRESULT Direct3DDevice::DeleteMatrix(
	D3DMATRIXHANDLE d3dMatHandle
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

HRESULT Direct3DDevice::BeginScene()
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	if (!this->_deviceResources->_renderTargetView)
	{
#if LOGGER
		str.str("\tD3DERR_SCENE_BEGIN_FAILED");
		LogText(str.str());
#endif

		return D3DERR_SCENE_BEGIN_FAILED;
	}

	this->_deviceResources->inScene = true;
	this->_deviceResources->inSceneBackbufferLocked = false;

	auto& context = this->_deviceResources->_d3dDeviceContext;

	if (!this->_deviceResources->sceneRendered || this->_deviceResources->clearColorSet)
	{
		// Clear only directly after flip
		context->ClearRenderTargetView(this->_deviceResources->_renderTargetView, this->_deviceResources->clearColor);
		this->_deviceResources->clearColorSet = false;
	}
	if (!this->_deviceResources->sceneRendered || this->_deviceResources->clearDepthSet)
	{
	    context->ClearDepthStencilView(this->_deviceResources->_depthStencilView, D3D11_CLEAR_DEPTH, this->_deviceResources->clearDepth, 0);
		this->_deviceResources->clearDepthSet = false;
    }

	if (FAILED(this->_deviceResources->RenderMain(this->_deviceResources->_backbufferSurface->_buffer, this->_deviceResources->_displayWidth, this->_deviceResources->_displayHeight, this->_deviceResources->_displayBpp)))
		return D3DERR_SCENE_BEGIN_FAILED;

	if (this->_deviceResources->_displayBpp == 2)
	{
		unsigned short* buffer = (unsigned short*)this->_deviceResources->_backbufferSurface->_buffer;
		int length = this->_deviceResources->_displayWidth * this->_deviceResources->_displayHeight;

		int i;
		for (i = 64; i <= length; i += 64)
		{
			_mm_storeu_si128((__m128i *)(buffer + i - 64), _mm_set1_epi16(0x2000));
			_mm_storeu_si128((__m128i *)(buffer + i - 56), _mm_set1_epi16(0x2000));
			_mm_storeu_si128((__m128i *)(buffer + i - 48), _mm_set1_epi16(0x2000));
			_mm_storeu_si128((__m128i *)(buffer + i - 40), _mm_set1_epi16(0x2000));
			_mm_storeu_si128((__m128i *)(buffer + i - 32), _mm_set1_epi16(0x2000));
			_mm_storeu_si128((__m128i *)(buffer + i - 24), _mm_set1_epi16(0x2000));
			_mm_storeu_si128((__m128i *)(buffer + i - 16), _mm_set1_epi16(0x2000));
			_mm_storeu_si128((__m128i *)(buffer + i - 8), _mm_set1_epi16(0x2000));
		}
		i -= 64;
		for (; i < length; i++)
		{
			buffer[i] = 0x2000;
		}
	}
	else
	{
		unsigned int* buffer = (unsigned int*)this->_deviceResources->_backbufferSurface->_buffer;
		int length = this->_deviceResources->_displayWidth * this->_deviceResources->_displayHeight;
		int i;
		for (i = 32; i <= length; i += 32)
		{
			_mm_storeu_si128((__m128i *)(buffer + i - 32), _mm_set1_epi32(0x200000));
			_mm_storeu_si128((__m128i *)(buffer + i - 28), _mm_set1_epi32(0x200000));
			_mm_storeu_si128((__m128i *)(buffer + i - 24), _mm_set1_epi32(0x200000));
			_mm_storeu_si128((__m128i *)(buffer + i - 20), _mm_set1_epi32(0x200000));
			_mm_storeu_si128((__m128i *)(buffer + i - 16), _mm_set1_epi32(0x200000));
			_mm_storeu_si128((__m128i *)(buffer + i - 12), _mm_set1_epi32(0x200000));
			_mm_storeu_si128((__m128i *)(buffer + i - 8), _mm_set1_epi32(0x200000));
			_mm_storeu_si128((__m128i *)(buffer + i - 4), _mm_set1_epi32(0x200000));
		}
		i -= 32;
		for (; i < length; i++)
		{
			buffer[i] = 0x200000;
		}
	}

	return D3D_OK;
}

HRESULT Direct3DDevice::EndScene()
{
#if LOGGER
	std::ostringstream str;
	str << this << " " << __FUNCTION__;
	LogText(str.str());
#endif

	this->_deviceResources->sceneRendered = true;

	this->_deviceResources->inScene = false;

	return D3D_OK;
}

HRESULT Direct3DDevice::GetDirect3D(
	LPDIRECT3D *lplpD3D
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
