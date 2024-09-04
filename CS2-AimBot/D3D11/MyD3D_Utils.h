#pragma once
#include <d3d11.h>
#include <DirectXColors.h>
#include <cmath>
#include "../vector.h"

namespace MyD3D_Utils
{
	HWND FindMainWindow(DWORD dwPID);
	BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam);
	bool WorldToScreen(Vector_t pWorldPos, Vector_t& pScreenPos, float* pMatrixPtr, const FLOAT pWinWidth, const FLOAT pWinHeight);

	inline constexpr D3DCOLORVALUE red{ 1.0f, 0.0f, 0.0f, 1.0f };
	inline constexpr D3DCOLORVALUE green{ 0.0f, 1.0f, 0.0f, 1.0f };
	inline constexpr D3DCOLORVALUE blue{ 0.0f, 0.0f, 1.0f, 1.0f };
	inline constexpr D3DCOLORVALUE magenta{ 1.0f, 0.0f, 1.0f, 1.0f };
	inline constexpr D3DCOLORVALUE yellow{ 1.0f, 1.0f, 0.0f, 1.0f };
}

struct Vertex
{
	DirectX::XMFLOAT3 pos;
	D3DCOLORVALUE color;
};

struct WindowData
{
	DWORD wndId;
	HWND hWnd;
};