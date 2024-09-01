#pragma once
#include "entity.h"
#include "vector.h"
#include "qangle.h"
#include <Windows.h>
typedef struct window_size
{
	int width;
	int height;
};
bool isVisible(C_CSPlayerPawn* pentity, int hitbox_id);
C_CSPlayerPawn* get_localplaypawn_address();
bool WorldToScreen(const Vector_t& pos, Vector2D_t& topos);
HWND Get_Window_Handle();
window_size get_window_size();
Vector_t GetPositionScreen(int fov, QAngle_t AimPunchAngle);
void SetViewAngle(QAngle_t dstangle, QAngle_t sourceAngle);
