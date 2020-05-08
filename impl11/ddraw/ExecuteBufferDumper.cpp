// Copyright (c) 2014 Jérémy Ansel
// Licensed under the MIT license. See LICENSE.txt

#include "common.h"
#include "DeviceResources.h"
#include "Direct3DDevice.h"
#include "Direct3DExecuteBuffer.h"
#include "ExecuteBufferDumper.h"

#if LOGGER

bool DumpExecuteBufferHasTriangling(Direct3DExecuteBuffer* executeBuffer);

void DumpInstruction(std::ostringstream& str, LPD3DINSTRUCTION instruction);

void DumpInstructionPoint(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionLine(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionTriangle(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionMatrixLoad(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionMatrixMultiply(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionStateTransform(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionStateLight(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionStateRender(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionProcessVertices(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionTextureLoad(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionExit(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionBranchForward(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionSpan(std::ostringstream& str, LPD3DINSTRUCTION instruction);
void DumpInstructionSetStatus(std::ostringstream& str, LPD3DINSTRUCTION instruction);

void DumpExecuteBuffer(Direct3DExecuteBuffer* executeBuffer)
{
	std::ostringstream str;

	str << "\tExecute Buffer: ";

	if (DumpExecuteBufferHasTriangling(executeBuffer))
	{
		str << "triangling";
	}

	LPD3DTLVERTEX pVertex = (LPD3DTLVERTEX)(executeBuffer->_buffer + executeBuffer->_executeData.dwVertexOffset);

	for (DWORD index = 0; index < executeBuffer->_executeData.dwVertexCount; index++)
	{
		D3DTLVERTEX v = *pVertex;
		pVertex++;

		v.sx = v.sx;
		v.sy = v.sy;
		v.sz = 1.0f - v.sz;
		v.rhw = 1.0f;

		str << std::endl;
		str << "\t" << index << ":";
		str << "\t( " << v.sx << "\t; " << v.sy << "\t; " << v.sz << " )";
		str << "\t" << v.rhw;
		str << "\t " << (void*)v.color;
		str << "\t " << (void*)v.specular;
		str << "\t( " << v.tu << "\t; " << v.tv << " )";
	}

	char* pData = executeBuffer->_buffer + executeBuffer->_executeData.dwInstructionOffset;
	char* pDataEnd = pData + executeBuffer->_executeData.dwInstructionLength;
	DWORD instructionIndex = 0;

	while (pData < pDataEnd)
	{
		LPD3DINSTRUCTION instruction = (LPD3DINSTRUCTION)pData;

		str << std::endl;
		str << "\t" << instructionIndex << ":";

		DumpInstruction(str, instruction);

		pData += sizeof(D3DINSTRUCTION) + instruction->bSize * instruction->wCount;
		instructionIndex++;
	}

	LogText(str.str());
}

bool DumpExecuteBufferHasTriangling(Direct3DExecuteBuffer* executeBuffer)
{
	DWORD vertexCount = executeBuffer->_executeData.dwVertexCount;

	char* pData = executeBuffer->_buffer + executeBuffer->_executeData.dwInstructionOffset;
	char* pDataEnd = pData + executeBuffer->_executeData.dwInstructionLength;

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
				if (triangle->v1 >= vertexCount || triangle->v2 >= vertexCount || triangle->v3 >= vertexCount)
				{
					return true;
				}

				triangle++;
			}
		}
		}
	}

	return false;
}

void DumpInstruction(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{
	switch (instruction->bOpcode)
	{
	case D3DOP_POINT:
		str << "\tPOINT";
		DumpInstructionPoint(str, instruction);
		break;

	case D3DOP_LINE:
		str << "\tLINE";
		DumpInstructionLine(str, instruction);
		break;

	case D3DOP_TRIANGLE:
		str << "\tTRIANGLE";
		DumpInstructionTriangle(str, instruction);
		break;

	case D3DOP_MATRIXLOAD:
		str << "\tMATRIXLOAD";
		DumpInstructionMatrixLoad(str, instruction);
		break;

	case D3DOP_MATRIXMULTIPLY:
		str << "\tMATRIXMULTIPLY";
		DumpInstructionMatrixMultiply(str, instruction);
		break;

	case D3DOP_STATETRANSFORM:
		str << "\tSTATETRANSFORM";
		DumpInstructionStateTransform(str, instruction);
		break;

	case D3DOP_STATELIGHT:
		str << "\tSTATELIGHT";
		DumpInstructionStateLight(str, instruction);
		break;

	case D3DOP_STATERENDER:
		str << "\tSTATERENDER";
		DumpInstructionStateRender(str, instruction);
		break;

	case D3DOP_PROCESSVERTICES:
		str << "\tPROCESSVERTICES";
		DumpInstructionProcessVertices(str, instruction);
		break;

	case D3DOP_TEXTURELOAD:
		str << "\tTEXTURELOAD";
		DumpInstructionTextureLoad(str, instruction);
		break;

	case D3DOP_EXIT:
		str << "\tEXIT";
		DumpInstructionExit(str, instruction);
		break;

	case D3DOP_BRANCHFORWARD:
		str << "\tBRANCHFORWARD";
		DumpInstructionBranchForward(str, instruction);
		break;

	case D3DOP_SPAN:
		str << "\tSPAN";
		DumpInstructionSpan(str, instruction);
		break;

	case D3DOP_SETSTATUS:
		str << "\tSETSTATUS";
		DumpInstructionSetStatus(str, instruction);
		break;
	default:
		str << "\tUNKNOWN INSTR " << instruction->bOpcode;
	}
}

void DumpInstructionPoint(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

void DumpInstructionLine(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

void DumpInstructionTriangle(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{
	LPD3DTRIANGLE triangle = (LPD3DTRIANGLE)(instruction + 1);

	for (WORD i = 0; i < instruction->wCount; i++)
	{
		str << std::endl;
		str << "\t\t" << triangle->v1;
		str << "\t\t" << triangle->v2;
		str << "\t\t" << triangle->v3;

		triangle++;
	}
}

void DumpInstructionMatrixLoad(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

void DumpInstructionMatrixMultiply(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

void DumpInstructionStateTransform(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

void DumpInstructionStateLight(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

void DumpInstructionStateRender(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{
	LPD3DSTATE state = (LPD3DSTATE)(instruction + 1);

	for (WORD i = 0; i < instruction->wCount; i++)
	{
		str << std::endl;
		str << "\t\t";

		switch (state->drstRenderStateType)
		{
		case D3DRENDERSTATE_TEXTUREHANDLE:
			str << "TEXTUREHANDLE " << state->dwArg[0];
			break;

		case D3DRENDERSTATE_ANTIALIAS:
			str << "ANTIALIAS ";

			switch (state->dwArg[0])
			{
			case D3DANTIALIAS_NONE:
				str << "NONE";
				break;

			case D3DANTIALIAS_SORTDEPENDENT:
				str << "SORTDEPENDENT";
				break;

			case D3DANTIALIAS_SORTINDEPENDENT:
				str << "SORTINDEPENDENT";
				break;
			}
			break;

		case D3DRENDERSTATE_TEXTUREADDRESS:
			str << "TEXTUREADDRESS ";

			switch (state->dwArg[0])
			{
			case D3DTADDRESS_WRAP:
				str << "WRAP";
				break;

			case D3DTADDRESS_MIRROR:
				str << "MIRROR";
				break;

			case D3DTADDRESS_CLAMP:
				str << "CLAMP";
				break;

			case D3DTADDRESS_BORDER:
				str << "BORDER";
				break;
			}
			break;

		case D3DRENDERSTATE_TEXTUREPERSPECTIVE:
			str << "TEXTUREPERSPECTIVE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_WRAPU:
			str << "WRAPU " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_WRAPV:
			str << "WRAPV " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_ZENABLE:
			str << "ZENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_FILLMODE:
			str << "FILLMODE ";

			switch (state->dwArg[0])
			{
			case D3DFILL_POINT:
				str << "POINT";
				break;

			case D3DFILL_WIREFRAME:
				str << "WIREFRAME";
				break;

			case D3DFILL_SOLID:
				str << "SOLID";
				break;
			}
			break;

		case D3DRENDERSTATE_SHADEMODE:
			str << "SHADEMODE ";

			switch (state->dwArg[0])
			{
			case D3DSHADE_FLAT:
				str << "FLAT";
				break;

			case D3DSHADE_GOURAUD:
				str << "GOURAUD";
				break;

			case D3DSHADE_PHONG:
				str << "PHONG";
				break;
			}
			break;

		case D3DRENDERSTATE_LINEPATTERN:
			str << "LINEPATTERN";
			break;

		case D3DRENDERSTATE_MONOENABLE:
			str << "MONOENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_ROP2:
			str << "ROP2";
			break;

		case D3DRENDERSTATE_PLANEMASK:
			str << "PLANEMASK";
			break;

		case D3DRENDERSTATE_ZWRITEENABLE:
			str << "ZWRITEENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_ALPHATESTENABLE:
			str << "ALPHATESTENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_LASTPIXEL:
			str << "LASTPIXEL " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_TEXTUREMAG:
			str << "TEXTUREMAG ";

			switch (state->dwArg[0])
			{
			case D3DFILTER_NEAREST:
				str << "NEAREST";
				break;

			case D3DFILTER_LINEAR:
				str << "LINEAR";
				break;

			case D3DFILTER_MIPNEAREST:
				str << "MIPNEAREST";
				break;

			case D3DFILTER_MIPLINEAR:
				str << "MIPLINEAR";
				break;

			case D3DFILTER_LINEARMIPNEAREST:
				str << "LINEARMIPNEAREST";
				break;

			case D3DFILTER_LINEARMIPLINEAR:
				str << "LINEARMIPLINEAR";
				break;
			}
			break;

		case D3DRENDERSTATE_TEXTUREMIN:
			str << "TEXTUREMIN ";

			switch (state->dwArg[0])
			{
			case D3DFILTER_NEAREST:
				str << "NEAREST";
				break;

			case D3DFILTER_LINEAR:
				str << "LINEAR";
				break;

			case D3DFILTER_MIPNEAREST:
				str << "MIPNEAREST";
				break;

			case D3DFILTER_MIPLINEAR:
				str << "MIPLINEAR";
				break;

			case D3DFILTER_LINEARMIPNEAREST:
				str << "LINEARMIPNEAREST";
				break;

			case D3DFILTER_LINEARMIPLINEAR:
				str << "LINEARMIPLINEAR";
				break;
			}
			break;

		case D3DRENDERSTATE_SRCBLEND:
			str << "SRCBLEND ";

			switch (state->dwArg[0])
			{
			case D3DBLEND_ZERO:
				str << "ZERO";
				break;

			case D3DBLEND_ONE:
				str << "ONE";
				break;

			case D3DBLEND_SRCCOLOR:
				str << "SRCCOLOR";
				break;

			case D3DBLEND_INVSRCCOLOR:
				str << "INVSRCCOLOR";
				break;

			case D3DBLEND_SRCALPHA:
				str << "SRCALPHA";
				break;

			case D3DBLEND_INVSRCALPHA:
				str << "INVSRCALPHA";
				break;

			case D3DBLEND_DESTALPHA:
				str << "DESTALPHA";
				break;

			case D3DBLEND_INVDESTALPHA:
				str << "INVDESTALPHA";
				break;

			case D3DBLEND_DESTCOLOR:
				str << "DESTCOLOR";
				break;

			case D3DBLEND_INVDESTCOLOR:
				str << "INVDESTCOLOR";
				break;

			case D3DBLEND_SRCALPHASAT:
				str << "SRCALPHASAT";
				break;

			case D3DBLEND_BOTHSRCALPHA:
				str << "BOTHSRCALPHA";
				break;

			case D3DBLEND_BOTHINVSRCALPHA:
				str << "BOTHINVSRCALPHA";
				break;
			}
			break;

		case D3DRENDERSTATE_DESTBLEND:
			str << "DESTBLEND ";

			switch (state->dwArg[0])
			{
			case D3DBLEND_ZERO:
				str << "ZERO";
				break;

			case D3DBLEND_ONE:
				str << "ONE";
				break;

			case D3DBLEND_SRCCOLOR:
				str << "SRCCOLOR";
				break;

			case D3DBLEND_INVSRCCOLOR:
				str << "INVSRCCOLOR";
				break;

			case D3DBLEND_SRCALPHA:
				str << "SRCALPHA";
				break;

			case D3DBLEND_INVSRCALPHA:
				str << "INVSRCALPHA";
				break;

			case D3DBLEND_DESTALPHA:
				str << "DESTALPHA";
				break;

			case D3DBLEND_INVDESTALPHA:
				str << "INVDESTALPHA";
				break;

			case D3DBLEND_DESTCOLOR:
				str << "DESTCOLOR";
				break;

			case D3DBLEND_INVDESTCOLOR:
				str << "INVDESTCOLOR";
				break;

			case D3DBLEND_SRCALPHASAT:
				str << "SRCALPHASAT";
				break;

			case D3DBLEND_BOTHSRCALPHA:
				str << "BOTHSRCALPHA";
				break;

			case D3DBLEND_BOTHINVSRCALPHA:
				str << "BOTHINVSRCALPHA";
				break;
			}
			break;

		case D3DRENDERSTATE_TEXTUREMAPBLEND:
			str << "TEXTUREMAPBLEND ";

			switch (state->dwArg[0])
			{
			case D3DTBLEND_DECAL:
				str << "DECAL";
				break;

			case D3DTBLEND_MODULATE:
				str << "MODULATE";
				break;

			case D3DTBLEND_DECALALPHA:
				str << "DECALALPHA";
				break;

			case D3DTBLEND_MODULATEALPHA:
				str << "MODULATEALPHA";
				break;

			case D3DTBLEND_DECALMASK:
				str << "DECALMASK";
				break;

			case D3DTBLEND_MODULATEMASK:
				str << "MODULATEMASK";
				break;

			case D3DTBLEND_COPY:
				str << "COPY";
				break;

			case D3DTBLEND_ADD:
				str << "ADD";
				break;
			}
			break;

		case D3DRENDERSTATE_CULLMODE:
			str << "CULLMODE ";

			switch (state->dwArg[0])
			{
			case D3DCULL_NONE:
				str << "NONE";
				break;

			case D3DCULL_CW:
				str << "CW";
				break;

			case D3DCULL_CCW:
				str << "CCW";
				break;
			}
			break;

		case D3DRENDERSTATE_ZFUNC:
			str << "ZFUNC ";

			switch (state->dwArg[0])
			{
			case D3DCMP_NEVER:
				str << "NEVER";
				break;

			case D3DCMP_LESS:
				str << "LESS";
				break;

			case D3DCMP_EQUAL:
				str << "EQUAL";
				break;

			case D3DCMP_LESSEQUAL:
				str << "LESSEQUAL";
				break;

			case D3DCMP_GREATER:
				str << "GREATER";
				break;

			case D3DCMP_NOTEQUAL:
				str << "NOTEQUAL";
				break;

			case D3DCMP_GREATEREQUAL:
				str << "GREATEREQUAL";
				break;

			case D3DCMP_ALWAYS:
				str << "ALWAYS";
				break;
			}
			break;

		case D3DRENDERSTATE_ALPHAREF:
			str << "ALPHAREF";
			break;

		case D3DRENDERSTATE_ALPHAFUNC:
			str << "ALPHAFUNC ";

			switch (state->dwArg[0])
			{
			case D3DCMP_NEVER:
				str << "NEVER";
				break;

			case D3DCMP_LESS:
				str << "LESS";
				break;

			case D3DCMP_EQUAL:
				str << "EQUAL";
				break;

			case D3DCMP_LESSEQUAL:
				str << "LESSEQUAL";
				break;

			case D3DCMP_GREATER:
				str << "GREATER";
				break;

			case D3DCMP_NOTEQUAL:
				str << "NOTEQUAL";
				break;

			case D3DCMP_GREATEREQUAL:
				str << "GREATEREQUAL";
				break;

			case D3DCMP_ALWAYS:
				str << "ALWAYS";
				break;
			}
			break;

		case D3DRENDERSTATE_DITHERENABLE:
			str << "DITHERENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_FOGENABLE:
			str << "FOGENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_SPECULARENABLE:
			str << "SPECULARENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_ZVISIBLE:
			str << "ZVISIBLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_SUBPIXEL:
			str << "SUBPIXEL " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_SUBPIXELX:
			str << "SUBPIXELX " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_STIPPLEDALPHA:
			str << "STIPPLEDALPHA " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_FOGCOLOR:
			str << "FOGCOLOR " << (void*)state->dwArg[0];
			break;

		case D3DRENDERSTATE_FOGTABLEMODE:
			str << "FOGTABLEMODE ";

			switch (state->dwArg[0])
			{
			case D3DFOG_NONE:
				str << "NONE";
				break;

			case D3DFOG_EXP:
				str << "EXP";
				break;

			case D3DFOG_EXP2:
				str << "EXP2";
				break;

			case D3DFOG_LINEAR:
				str << "LINEAR";
				break;
			}
			break;

		case D3DRENDERSTATE_FOGTABLESTART:
			str << "FOGTABLESTART " << state->dvArg[0];
			break;

		case D3DRENDERSTATE_FOGTABLEEND:
			str << "FOGTABLEEND " << state->dvArg[0];
			break;

		case D3DRENDERSTATE_FOGTABLEDENSITY:
			str << "FOGTABLEDENSITY " << state->dvArg[0];
			break;

		case D3DRENDERSTATE_STIPPLEENABLE:
			str << "STIPPLEENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_EDGEANTIALIAS:
			str << "EDGEANTIALIAS " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_COLORKEYENABLE:
			str << "COLORKEYENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_ALPHABLENDENABLE:
			str << "ALPHABLENDENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_BORDERCOLOR:
			str << "BORDERCOLOR " << (void*)state->dwArg[0];
			break;

		case D3DRENDERSTATE_TEXTUREADDRESSU:
			str << "TEXTUREADDRESSU ";

			switch (state->dwArg[0])
			{
			case D3DTADDRESS_WRAP:
				str << "WRAP";
				break;

			case D3DTADDRESS_MIRROR:
				str << "MIRROR";
				break;

			case D3DTADDRESS_CLAMP:
				str << "CLAMP";
				break;

			case D3DTADDRESS_BORDER:
				str << "BORDER";
				break;
			}
			break;

		case D3DRENDERSTATE_TEXTUREADDRESSV:
			str << "TEXTUREADDRESSV ";

			switch (state->dwArg[0])
			{
			case D3DTADDRESS_WRAP:
				str << "WRAP";
				break;

			case D3DTADDRESS_MIRROR:
				str << "MIRROR";
				break;

			case D3DTADDRESS_CLAMP:
				str << "CLAMP";
				break;

			case D3DTADDRESS_BORDER:
				str << "BORDER";
				break;
			}
			break;

		case D3DRENDERSTATE_MIPMAPLODBIAS:
			str << "MIPMAPLODBIAS " << state->dvArg[0];
			break;

		case D3DRENDERSTATE_ZBIAS:
			str << "ZBIAS " << state->dwArg[0];
			break;

		case D3DRENDERSTATE_RANGEFOGENABLE:
			str << "RANGEFOGENABLE " << (state->dwArg[0] ? "TRUE" : "FALSE");
			break;

		case D3DRENDERSTATE_ANISOTROPY:
			str << "ANISOTROPY " << state->dwArg[0];
			break;

		case D3DRENDERSTATE_STIPPLEPATTERN00:
		case D3DRENDERSTATE_STIPPLEPATTERN01:
		case D3DRENDERSTATE_STIPPLEPATTERN02:
		case D3DRENDERSTATE_STIPPLEPATTERN03:
		case D3DRENDERSTATE_STIPPLEPATTERN04:
		case D3DRENDERSTATE_STIPPLEPATTERN05:
		case D3DRENDERSTATE_STIPPLEPATTERN06:
		case D3DRENDERSTATE_STIPPLEPATTERN07:
		case D3DRENDERSTATE_STIPPLEPATTERN08:
		case D3DRENDERSTATE_STIPPLEPATTERN09:
		case D3DRENDERSTATE_STIPPLEPATTERN10:
		case D3DRENDERSTATE_STIPPLEPATTERN11:
		case D3DRENDERSTATE_STIPPLEPATTERN12:
		case D3DRENDERSTATE_STIPPLEPATTERN13:
		case D3DRENDERSTATE_STIPPLEPATTERN14:
		case D3DRENDERSTATE_STIPPLEPATTERN15:
		case D3DRENDERSTATE_STIPPLEPATTERN16:
		case D3DRENDERSTATE_STIPPLEPATTERN17:
		case D3DRENDERSTATE_STIPPLEPATTERN18:
		case D3DRENDERSTATE_STIPPLEPATTERN19:
		case D3DRENDERSTATE_STIPPLEPATTERN20:
		case D3DRENDERSTATE_STIPPLEPATTERN21:
		case D3DRENDERSTATE_STIPPLEPATTERN22:
		case D3DRENDERSTATE_STIPPLEPATTERN23:
		case D3DRENDERSTATE_STIPPLEPATTERN24:
		case D3DRENDERSTATE_STIPPLEPATTERN25:
		case D3DRENDERSTATE_STIPPLEPATTERN26:
		case D3DRENDERSTATE_STIPPLEPATTERN27:
		case D3DRENDERSTATE_STIPPLEPATTERN28:
		case D3DRENDERSTATE_STIPPLEPATTERN29:
		case D3DRENDERSTATE_STIPPLEPATTERN30:
		case D3DRENDERSTATE_STIPPLEPATTERN31:
			str << "STIPPLEPATTERNxx";
			break;
		default:
			str << "UNKNOWN STATE " << state->drstRenderStateType;
		}

		state++;
	}
}

void DumpInstructionProcessVertices(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{
	LPD3DPROCESSVERTICES process = (LPD3DPROCESSVERTICES)(instruction + 1);

	for (WORD i = 0; i < instruction->wCount; i++)
	{
		str << std::endl;
		str << "\t\t";

		switch (process->dwFlags & D3DPROCESSVERTICES_OPMASK)
		{
		case D3DPROCESSVERTICES_TRANSFORMLIGHT:
			str << "TRANSFORMLIGHT";
			break;

		case D3DPROCESSVERTICES_TRANSFORM:
			str << "TRANSFORM";
			break;

		case D3DPROCESSVERTICES_COPY:
			str << "COPY";
			break;
		}

		if (process->dwFlags & D3DPROCESSVERTICES_UPDATEEXTENTS)
		{
			str << " UPDATEEXTENTS";
		}

		if (process->dwFlags & D3DPROCESSVERTICES_NOCOLOR)
		{
			str << " NOCOLOR";
		}

		str << "\tstart: " << process->wStart << "\tdest: " << process->wDest << "\tcount:" << process->dwCount;

		process++;
	}
}

void DumpInstructionTextureLoad(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

void DumpInstructionExit(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

void DumpInstructionBranchForward(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

void DumpInstructionSpan(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

void DumpInstructionSetStatus(std::ostringstream& str, LPD3DINSTRUCTION instruction)
{

}

#endif // LOGGER
