#include "hooks.h"
#include "csgoinput.h"
#include "iengineclient.h"
#include "cgameresourceservice.h"
#include "cgameentitysystem.h"
#include "entity.h"
// used: get virtual function, find pattern, ...
#include "memory.h"
#include <iostream>
#include <stdio.h>
#include "aim.h"
#include <functional>
#include "safetyhook.hpp"
#include "Zydis.h"
#include "iswapchaindx11.h"
#include "esp.h"
#include "D3D11/MyD3d11.h"
#include "global.hpp"


extern HWND hWindow;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
MyD3D11 g_myD3d11{};
static WNDPROC oWndProc = nullptr;


safetyhook::MidHook g_createmove_hook{};
safetyhook::InlineHook g_present_hook{};
safetyhook::InlineHook g_resizebuffers_hook{};
safetyhook::InlineHook g_createswapchain_hook{};


void InitImGui()
{
	ImGui::CreateContext();

	auto io = ImGui::GetIO();
	auto& style = ImGui::GetStyle();

	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	style.WindowMinSize = ImVec2(256, 300);
	style.WindowTitleAlign = ImVec2(0.5, 0.5);
	style.FrameBorderSize = 1;
	style.ChildBorderSize = 1;
	style.WindowBorderSize = 1;
	style.WindowRounding = 0;
	style.FrameRounding = 0;
	style.ChildRounding = 0;
	style.Colors[ImGuiCol_TitleBg] = ImColor(70, 70, 70);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(70, 70, 70);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(70, 70, 70);
	style.Colors[ImGuiCol_WindowBg] = ImColor(25, 25, 25, 240);
	style.Colors[ImGuiCol_CheckMark] = ImColor(70, 70, 70);
	style.Colors[ImGuiCol_Border] = ImColor(70, 70, 70);
	style.Colors[ImGuiCol_Button] = ImColor(32, 32, 32);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_ChildBg] = ImColor(45, 45, 45);
	style.Colors[ImGuiCol_FrameBg] = ImColor(32, 32, 32);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(42, 42, 42);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(255, 255, 255);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(255, 255, 255);

	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 14.0f);

	ImGui_ImplWin32_Init(hWindow);
	ImGui_ImplDX11_Init(I::Device, I::DeviceContext);
}


LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (uMsg == WM_KEYUP && wParam == VK_HOME)
		Globals::Open ^= 1;

	if (Globals::Open)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}



void  hooked_CreateMove(SafetyHookContext& ctx) {

	CCSGOInput* input = *reinterpret_cast<CCSGOInput**>(ctx.rsp + 0x8);
	int nslot = *reinterpret_cast<int*>(ctx.rsp + 0x10);
	bool bative = *reinterpret_cast<bool*>(ctx.rsp + 0x18);
	bool rax = ctx.rax;
	H::CreateMove(input, nslot, bative, rax);
}

void* get_ip_address(uintptr_t base_address)
{
	ZydisDecoder decoder{};

	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

	

	//g_add_hook = safetyhook::create_inline(pCreateMoveOrignal, reinterpret_cast<void*>(&CreateMove));

	auto ip = reinterpret_cast<uint8_t*>(base_address);

	while (*ip != 0xC3) {
		ZydisDecodedInstruction ix{};

		ZydisDecoderDecodeInstruction(&decoder, nullptr, reinterpret_cast<void*>(ip), 15, &ix);

		// Follow JMPs
		if (ix.opcode == 0xE9) {
			ip += ix.length + (int32_t)ix.raw.imm[0].value.s;
		}
		else {
			ip += ix.length;
		}
	}
	return ip;
}

bool H::Setup()
{


	



	

	auto pCreateMoveOrignal = MEM::GetVFunc(I::Input, VTABLE::CLIENT::CREATEMOVE);
	
	void* createmove_ip = get_ip_address((uintptr_t)pCreateMoveOrignal);


	
	g_createmove_hook = safetyhook::create_mid(createmove_ip, hooked_CreateMove);


	

	g_present_hook = safetyhook::create_inline(MEM::GetVFunc(I::SwapChain->pDXGISwapChain, VTABLE::D3D::PRESENT), &Present);
	
	g_resizebuffers_hook = safetyhook::create_inline(MEM::GetVFunc(I::SwapChain->pDXGISwapChain, VTABLE::D3D::RESIZEBUFFERS), &ResizeBuffers);

	IDXGIDevice* pDXGIDevice = NULL;
	I::Device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice));

	IDXGIAdapter* pDXGIAdapter = NULL;
	pDXGIDevice->GetAdapter(&pDXGIAdapter);

	IDXGIFactory* pIDXGIFactory = NULL;
	pDXGIAdapter->GetParent(IID_PPV_ARGS(&pIDXGIFactory));

	g_createswapchain_hook = safetyhook::create_inline(MEM::GetVFunc(pIDXGIFactory, VTABLE::DXGI::CREATESWAPCHAIN), &CreateSwapChain);


	

	

	pDXGIDevice->Release();
	pDXGIDevice = nullptr;
	pDXGIAdapter->Release();
	pDXGIAdapter = nullptr;
	pIDXGIFactory->Release();
	pIDXGIFactory = nullptr;

	
	// @ida: #STR: cl: CreateMove clamped invalid attack history index %d in frame history to -1. Was %d, frame history size %d.\n
	/*if (!hkCreateMove.Create(MEM::GetVFunc(I::Input, VTABLE::CLIENT::CREATEMOVE), reinterpret_cast<void*>(&CreateMove)))
		return false;*/
	

	

	// @note: seems to do nothing for now...
	// @ida: ClientModeCSNormal->OverrideView idx 15
	//v21 = flSomeWidthSize * 0.5;
	//v22 = *flSomeHeightSize * 0.5;
	//*(float*)(pSetup + 0x49C) = v21; // m_OrthoRight
	//*(float*)(pSetup + 0x494) = -v21; // m_OrthoLeft
	//*(float*)(pSetup + 0x498) = -v22; // m_OrthoTop
	//*(float*)(pSetup + 0x4A0) = v22; // m_OrthoBottom
	//if (!hkOverrideView.Create(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B FA E8")), reinterpret_cast<void*>(&OverrideView)))
	//	return false;

	//L_PRINT(LOG_INFO) << CS_XOR("\"OverrideView\" hook has been created");

	 

	return true;
}

void H::Destroy()
{
	oWndProc = (WNDPROC)SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
	g_createmove_hook.~MidHook();
	g_present_hook.reset();
	g_resizebuffers_hook.reset();
	g_createswapchain_hook.reset();
	 g_myD3d11.~MyD3D11();
}









void RCS_OnCreateMove(CCSGOInput* pCsgoInput, CUserCmd* pUserCmd,

	QAngle_t* view_angles, CCSPlayerController* pLocalController);





bool __fastcall H::CreateMove(CCSGOInput* pInput, int nSlot, bool bActive,bool bResult)
{
	//const auto oCreateMove = hkCreateMove.GetOriginal();
	//const bool bResult = oCreateMove(pInput, nSlot, bActive);
	//bool bResult = veh::CallOriginal<bool>(CreateMove, pInput, nSlot, bActive);
	
	if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
		return bResult;

	

	CUserCmd* pCmd = pInput->GetUserCmd();
	if (pCmd == nullptr)
		return bResult;
	CBaseUserCmdPB* pBaseCmd = pCmd->csgoUserCmd.pBaseCmd;
	if (pBaseCmd == nullptr) 
		return bResult;

	

	
	
	/*const std::uintptr_t dwLocalPlayerController = 0x19B8158;
	uintptr_t LocalControllerAddress = *reinterpret_cast<uintptr_t*>((uintptr_t)GetModuleHandleA("client.dll") + dwLocalPlayerController);*/
	const int nIndex = I::Engine->GetLocalPlayer();
	uintptr_t LocalControllerAddress = (uintptr_t)I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(nIndex);
	
	CCSPlayerController LocalController(LocalControllerAddress);

	if (LocalControllerAddress == NULL)
	{
		printf("LocalPlay is null\n");
		return bResult;
		
	}
	
		
	
	
	auto Pawn = LocalController.GetPawnHandle();
	  

	

    uintptr_t LocalPawnaddress = (uintptr_t)I::GameResourceService->pGameEntitySystem->Get<uintptr_t>(Pawn);
	C_CSPlayerPawn LocalPawn(LocalPawnaddress);

	if (LocalPawnaddress == NULL)
	{
		printf("LocalPawn is nullptr");
		return bResult;
	}
		 
	RCS_OnCreateMove(I::Input, pCmd, &(pBaseCmd->pViewAngles->angValue), &LocalController);
	F::OnCreateMove(pCmd, pBaseCmd, &LocalController);



	
	return bResult;
}



void OnDraw()
{
	static auto Slider = [&](const char* label, float* value, float min, float max, float width = ImGui::GetContentRegionAvail().x)
		{
			ImGui::PushID(label);
			ImGui::PushItemWidth(width);
			ImGui::SliderFloat("##CustomSliderF_", value, min, max);
			ImGui::PopItemWidth();
			ImGui::PopID();
			ImGui::Spacing();
		};

	static auto Checkbox = [&](const char* label, bool* value)
		{
			ImGui::PushID(label);
			ImGui::Checkbox(label, value);
			ImGui::PopID();
			ImGui::Spacing();
		};

	if (Globals::Open) {
		ImGui::Begin("D3D11 Hook", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
		{
			if (ImGui::Button("Aimbot", ImVec2(ImGui::GetContentRegionAvail().x / 4, 29)))
				Globals::Tab = 0;

			ImGui::SameLine();

			if (ImGui::Button("Visuals", ImVec2(ImGui::GetContentRegionAvail().x / 3, 29)))
				Globals::Tab = 1;

			ImGui::SameLine();

			if (ImGui::Button("Other", ImVec2(ImGui::GetContentRegionAvail().x / 2, 29)))
				Globals::Tab = 2;

			ImGui::SameLine();

			if (ImGui::Button("Settings", ImVec2(ImGui::GetContentRegionAvail().x / 1, 29)))
				Globals::Tab = 3;

			ImGui::Spacing();
			ImGui::Separator();

			switch (Globals::Tab)
			{
			case 0: // Aimbot
				Checkbox("Aimbot Enabled", &Globals::Aimbot::Aim);
				Checkbox("TriggerBot Enabled", &Globals::Aimbot::TriggerBot);
				Checkbox("ByMouse Enabled", &Globals::Aimbot::ByMouse);
				break;

			case 1: // Visuals
				Checkbox("ESP Enabled", &Globals::Visuals::Enabled);
				break;

			case 2: // Other
				break;

			case 3: // Settings
				if (ImGui::Button("Unhook"))
					Globals::IsClosing = true;
				break;
			}
		}
		ImGui::End();
	}
	if (Globals::Visuals::Enabled)
	{
		do_esp();
	}
}



HRESULT __stdcall H::Present(IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags)
{








	if (I::RenderTargetView == nullptr)
		I::CreateRenderTarget();

	static bool Init = false;
	if (!Init)
	{
		

			
			oWndProc = (WNDPROC)SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			Init = true;
		
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::GetIO().MouseDrawCursor = Globals::Open;
	OnDraw();

	ImGui::Render();

	if (I::RenderTargetView != nullptr)
	{
		I::DeviceContext->OMSetRenderTargets(1, &I::RenderTargetView, NULL);
	}
	
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
	

	return g_present_hook.stdcall<HRESULT>(I::SwapChain->pDXGISwapChain, uSyncInterval, uFlags);
}


HRESULT CS_FASTCALL H::ResizeBuffers(IDXGISwapChain* pSwapChain, std::uint32_t nBufferCount, std::uint32_t nWidth, std::uint32_t nHeight, DXGI_FORMAT newFormat, std::uint32_t nFlags)
{
	

	auto hResult = g_resizebuffers_hook.fastcall<HRESULT>(pSwapChain, nBufferCount, nWidth, nHeight, newFormat, nFlags);
	if (SUCCEEDED(hResult))
		I::CreateRenderTarget();

	return hResult;
}

HRESULT __stdcall H::CreateSwapChain(IDXGIFactory* pFactory, IUnknown* pDevice, DXGI_SWAP_CHAIN_DESC* pDesc, IDXGISwapChain** ppSwapChain)
{
	
	I::DestroyRenderTarget();
	

	return g_createswapchain_hook.stdcall<HRESULT>(pFactory, pDevice, pDesc, ppSwapChain);
}