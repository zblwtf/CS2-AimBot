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
	
	static bool IsClosing = false;
	static bool Open = true;
	static int Tab = 0;

	namespace Aimbot
	{
		static bool Aim = false;
		static bool ByMouse = false;
		static bool TriggerBot = true;
	}

	namespace Visuals
	{
		static bool Enabled = false;
		static bool Boxes = false;
		static bool Corners = false;
		static bool Snaplines = false;
		static bool FilledBoxes = false;
		static bool DisplayInfo = false;
		static bool DisplayHealth = false;
		static bool DisplayNames = false;
	}
}