#include "tool.h"
#include "cgametracemanager.h"
#include <Windows.h>
#include "entity.h"
#include "CView.h"
#include "offset.hpp"
#include "interfaces.h"
#include "vector.h"
#include "Timer.hpp"
extern HWND hWindow; 

extern uintptr_t client_address;
extern int screenWidth;
extern int screenHeight;
window_size get_window_size()
{
	window_size sz;
	RECT rct{};
	
	GetClientRect(hWindow, &rct);


	sz.width = (int)rct.right;
	sz.height = (int)rct.bottom;
	return sz;
}

C_CSPlayerPawn* get_localplaypawn_address()
{
	const uintptr_t dwLocalPlayerPawn = 0x17D3810;
	C_CSPlayerPawn* pLocalPawn = *reinterpret_cast<C_CSPlayerPawn**>(client_address+ dwLocalPlayerPawn);
	return pLocalPawn;
}
bool WorldToScreen(const Vector_t& pos, Vector2D_t& topos)
{
	CView pMatrix = *reinterpret_cast<CView*>(client_address + cs2_dumper::offsets::client_dll::dwViewMatrix);
	
	return pMatrix.WorldToScreen(pos, topos, screenWidth, screenHeight);
}
bool isVisible(C_CSPlayerPawn* pentity, int hitbox_id)
{
	GameTrace_t trace = GameTrace_t();
	TraceFilter_t filter = TraceFilter_t(0x1C3003, pentity, nullptr, 4);
	Ray_t ray = Ray_t();

	C_CSPlayerPawn* pLocalPawn = get_localplaypawn_address();


	// cast a ray from local player eye positon -> player head bone
	// @note: would recommend checking for nullptrs
	I::GameTraceManager->TraceShape(&ray, pLocalPawn->GetEyePosition(), pentity->BoneData.BonePosList[hitbox_id].Pos, &filter, &trace);
	// check if the hit entity is the one we wanted to check and if the trace end point is visible
	if (trace.m_pHitEntity != pentity || !trace.IsVisible())// if invisible, skip this entity
		return false;
	return true;
}

typedef struct CViewMatrix
{
	float Matrix[16];
};

CViewMatrix GetMatrixViewport()
{

	int width = screenWidth;
	int height = screenHeight;
	return
	{ width * 0.5f, 0, 0, 0,
		0, -(height * 0.5f), 0, 0,
		0, 0, 1, 0,
		width * 0.5f, height * 0.5f, 0, 1
	};
}
double DegreeToRadian(double degree) {
	return degree * 3.1415926575 / 180.0;
}

Vector_t ViewportTransform(CViewMatrix m, Vector_t v)
{
	float w = 1.0 / (m.Matrix[4 - 1] * v.x + m.Matrix[2 * 4 - 1] * v.y + m.Matrix[3 * 4 - 1] * v.z + m.Matrix[4 * 4 - 1]);
	auto x_ = m.Matrix[0] * v.x + m.Matrix[0 + 4] * v.y + m.Matrix[0 + 8] * v.z + m.Matrix[8 + 4] * w;
	auto y_ = m.Matrix[1] * v.x + m.Matrix[1 + 4] * v.y + m.Matrix[1 + 8] * v.z + m.Matrix[8 + 4 + 1] * w;
	auto z_ = m.Matrix[2] * v.x + m.Matrix[2 + 4] * v.y + m.Matrix[2 + 8] * v.z + m.Matrix[8 + 4 + 2] * w;
	return { x_, y_, z_ };
}


Vector_t GetPositionScreen(int fov, QAngle_t AimPunchAngle)
{
	static CViewMatrix playViewMatrix = GetMatrixViewport();

	auto spectRatio = (double)screenWidth / screenHeight;


	

	auto fovY = DegreeToRadian(fov);
	auto fovX = fovY * spectRatio;
	auto punchX = DegreeToRadian((double)AimPunchAngle.x * 2.0);
	auto punchY = DegreeToRadian((double)AimPunchAngle.y * 2.0);
	auto pointClip = Vector_t
	(
		(float)(-punchY / fovX),
		(float)(-punchX / fovY),
		0
	);

	return ViewportTransform(playViewMatrix, pointClip);
}

void MoveMouse(Vector2D_t point)
{

	mouse_event(MOUSEEVENTF_MOVE, (INT)point.x, (INT)point.y, 0, NULL);
	/*if (x_address > 0 && y_address > 0)
	{
		driver::write_memory(ProcessMgr.driver_handle, x_address, point.x);
		driver::write_memory(ProcessMgr.driver_handle, y_address, point.y);
	}*/
}

Vector_t AngleToPixels(QAngle_t angle, int fov, int width, int height)
{
	const float AnglePerPixel = 0.12;
	float aspectRatio = (float)width / height;
	float fov_Y = fov * (3.1415926535 / 180);
	float fov_X = fov_Y * aspectRatio;


	auto fovRatio = 90.0 / fov;
	float Yaw_pixel = roundf(angle.x / AnglePerPixel * fovRatio);
	float Pitch_pixel = roundf(angle.y / AnglePerPixel * fovRatio);
	return{ Pitch_pixel + fov_X,-Yaw_pixel + fov_Y,0 };
}
void SetViewAngle(QAngle_t dstangle, QAngle_t sourceAngle)
{
	static bool hhh = true;
	static Timer m_zoomTimer;
	if (hhh)
	{
		m_zoomTimer.start(1);
		hhh = false;
	}
	else
	{
		if (!m_zoomTimer.isElapsed())
		{
			return;
		}
		else
		{

			

			auto angle = sourceAngle - dstangle;
			angle.Clamp();
			auto screen_pixels = AngleToPixels(angle, 70, screenWidth, screenHeight);
			MoveMouse({ screen_pixels.x,screen_pixels.y });

			hhh = true;
		}
	}

}