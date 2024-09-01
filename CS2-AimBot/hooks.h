#pragma once

// used: [d3d] api
#include <d3d11.h>
#include <dxgi1_2.h>
#include <cstdint>
// used: chookobject
#include "detourhook.h"



namespace VTABLE
{
	namespace D3D
	{
		enum
		{
			PRESENT = 8U,
			RESIZEBUFFERS = 13U,
			RESIZEBUFFERS_CSTYLE = 39U,
		};
	}

	namespace DXGI
	{
		enum
		{
			CREATESWAPCHAIN = 10U,
		};
	}

	namespace CLIENT
	{
		enum
		{
			CREATEMOVE = 5U,
			MOUSEINPUTENABLED = 16U,
			FRAMESTAGENOTIFY = 36U,
		};
	}

	namespace INPUTSYSTEM
	{
		enum
		{
			ISRELATIVEMOUSEMODE = 78U,
		};
	}
}
class CRenderGameSystem;
class IViewRender;
class CCSGOInput;
class CViewSetup;
class CMeshData;

namespace H
{
	bool Setup();
	void Destroy();



	bool __fastcall CreateMove(CCSGOInput* pInput, int nSlot, bool bActive,bool rax);
	

	HRESULT __stdcall CreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain);
	HRESULT __stdcall Present(IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags);

	HRESULT CS_FASTCALL ResizeBuffers(IDXGISwapChain* pSwapChain, uint32_t nBufferCount, uint32_t nWidth, uint32_t nHeight, DXGI_FORMAT newFormat, uint32_t nFlags);
}
