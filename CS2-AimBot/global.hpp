#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <cstdio>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"





extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



namespace Globals
{
	extern bool Open;
	extern  int Tab;

	namespace Aimbot
	{
		extern bool Aim;
		extern  bool ByMouse;
		extern bool TriggerBot;
	}

	namespace Visuals
	{
		extern bool Enabled;
		
	}
}