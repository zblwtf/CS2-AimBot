#include "hooks.h"
#include "csgoinput.h"
#include "iengineclient.h"
#include "cgameresourceservice.h"
#include "cgameentitysystem.h"
#include "cgametracemanager.h"
#include "entity.h"
// used: get virtual function, find pattern, ...
#include "memory.h"
#include "aim.h"
#include "client.dll.hpp"
#include "fnv1a.h"
#include "tool.h"
#include <stdio.h>
#include <chrono>
bool trigger_bot_active = true;
extern uintptr_t client_address;
extern window_size window_info;
inline QAngle_t GetRecoil(CBaseUserCmdPB* pCmd, C_CSPlayerPawn* pLocal)
{
	static QAngle_t OldPunch;//get last tick AimPunch angles
	if (pLocal->GetShotsFired() >= 1)//only update aimpunch while shooting
	{
		QAngle_t viewAngles = pCmd->pViewAngles->angValue;
		QAngle_t delta = viewAngles - (viewAngles + (OldPunch - (pLocal->GetAimPunchAngle() * 2.f)));//get current AimPunch angles delta
			return pLocal->GetAimPunchAngle() * 2.0f;
		
	}
	else
	{
		return QAngle_t{ 0, 0 ,0 };//return 0 if is not shooting
	}
}

QAngle_t GetAngularDifference(CBaseUserCmdPB* pCmd, Vector_t vecTarget, C_CSPlayerPawn* pLocal)
{
	//// The current position
	//Vector_t vecCurrent = pLocal->GetEyePosition();

	//// The new angle
	//QAngle_t vNewAngle = (vecTarget - vecCurrent).ToAngles();
	//vNewAngle.Normalize(); // Normalise it so we don't jitter about

	//// Store our current angles
	//QAngle_t vCurAngle = pCmd->pViewAngles->angValue;

	//// Find the difference between the two angles (later useful when adding smoothing)
	//vNewAngle -= vCurAngle;

	//return vNewAngle;


	// The current position
	Vector_t vecCurrent = pLocal->GetEyePosition();

	// The new angle
	auto delta_vec = vecTarget - vecCurrent;


	

	float distance = sqrtf(powf(delta_vec.x, 2) + powf(delta_vec.y, 2));
	float new_pitch = 0.0, new_yaw = 0.0;

	new_yaw = atan2f(delta_vec.y, delta_vec.x) * (180 / 3.1415926535);
	new_pitch = -atan2f(delta_vec.z, distance) * (180 / 3.1415926535);
	QAngle_t vNewAngle = { new_pitch,new_yaw,0.0f };
	


	vNewAngle.Normalize(); // Normalise it so we don't jitter about

	// Store our current angles
	QAngle_t vCurAngle = pCmd->pViewAngles->angValue;

	// Find the difference between the two angles (later useful when adding smoothing)
	vNewAngle -= vCurAngle;

	return vNewAngle;
}

inline float GetAngularDistance(CBaseUserCmdPB* pCmd, Vector_t vecTarget, C_CSPlayerPawn* pLocal)
{
	return GetAngularDifference(pCmd, vecTarget, pLocal).Length2D();
}


constexpr auto deg2rad(float degrees) noexcept {
	return degrees * (3.1415926575 / 180.0f);
}

Vector_t FromAngle(const QAngle_t& angle) noexcept {
	return Vector_t{ std::cosf(deg2rad(angle.x)) * std::cosf(deg2rad(angle.y)),
					 std::cosf(deg2rad(angle.x)) * std::sinf(deg2rad(angle.y)),
					 -std::sinf(deg2rad(angle.x))
	};
}



void F::OnCreateMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController)
{
	
	C_CSPlayerPawn pLocalPawn((uintptr_t)I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalController->GetPawnHandle()));
	if (pLocalPawn.pthis == NULL)
		return;

	if (!pLocalController->GetIsAlive())
		return;


	

	// The current best distance
	float flDistance = INFINITY;
	static std::chrono::time_point LastTimePoint = std::chrono::steady_clock::now();
	// The target we have chosen
	static C_CSPlayerPawn BestTarget;
	// Cache'd position
	Vector_t vecBestPosition = Vector_t();

	// Entity loop
	const int iHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();

	//如果已经有目标了且目标没死那就不用更新目标了
	bool need_to_update_target = true;

	

	if (BestTarget.pthis != NULL)
	{
		if (BestTarget.GetHealth() > 0)
		{
			const int iBone = 6;
			Vector_t vecPos;
			BestTarget.get_bone_data(iBone, vecPos);
			GameTrace_t trace = GameTrace_t();
			TraceFilter_t filter = TraceFilter_t(0x1C3003, &pLocalPawn, nullptr, 4);
			Ray_t ray = Ray_t();

			// cast a ray from local player eye positon -> player head bone
			// @note: would recommend checking for nullptrs
			I::GameTraceManager->TraceShape(&ray, pLocalPawn.GetEyePosition(), vecPos, &filter, &trace);
			// check if the hit entity is the one we wanted to check and if the trace end point is visible
			if ((uintptr_t)trace.m_pHitEntity == BestTarget.pthis || trace.IsVisible())
			{
				need_to_update_target = false;
			}
		}

	}


	if (need_to_update_target)
	{
		BestTarget = C_CSPlayerPawn();

		LastTimePoint -= std::chrono::milliseconds(500);
		for (int nIndex = 1; nIndex <= iHighestIndex; nIndex++)
		{
			// Get the entity
			uintptr_t pEntity = (uintptr_t)I::GameResourceService->pGameEntitySystem->Get(nIndex);
			if (pEntity == NULL)
				continue;






			// Get the class info
			void* pClassInfo = nullptr;
			((CEntityInstance*)pEntity)->GetSchemaClassInfo(&pClassInfo);
			if (pClassInfo == nullptr)
				continue;
			char* pszName = *reinterpret_cast<char**>((uintptr_t)pClassInfo + 0x8);
			const FNV1A_t uHashedName = FNV1A::Hash(pszName);

			// Make sure they're a player controller
			if (uHashedName != FNV1A::HashConst("CCSPlayerController"))
				continue;


			//

			//// Cast to player controller
			CCSPlayerController pPlayer(pEntity);
			//
			//// Check the entity is not us
			if (pPlayer.Address == pLocalController->Address)
				continue;

			// Get the player pawn
			uintptr_t pPawn = (uintptr_t)I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer.GetPawnHandle());


			C_CSPlayerPawn playPawn(pPawn);
			if (pPawn == NULL)
				continue;

			// Make sure they're alive
			if (!pPlayer.GetIsAlive())
				continue;

			// Check if they're an enemy
			if (pLocalController->GetTeamID() == pPlayer.GetTeamID())
				continue;


			////C_BaseEntity->m_pGameSceneNode
			////CGameSceneNode::m_bDormant

			// Check if they're dormant
			uintptr_t pGameSceneNode = *reinterpret_cast<uintptr_t*>(pPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
			if (!pGameSceneNode)continue;
			if (*reinterpret_cast<bool*>(pGameSceneNode + cs2_dumper::schemas::client_dll::CGameSceneNode::m_bDormant))continue;


			//

			//
			const int iBone = 6; // You may wish to change this dynamically but for now let's target the head.

			// Get the bone's position

			Vector_t vecPos;
			playPawn.get_bone_data(iBone, vecPos);

			// @note: this is a simple example of how to check if the player is visible

			// initialize trace, construct filterr and initialize ray
			GameTrace_t trace = GameTrace_t();
			TraceFilter_t filter = TraceFilter_t(0x1C3003, &pLocalPawn, nullptr, 4);
			Ray_t ray = Ray_t();

			// cast a ray from local player eye positon -> player head bone
			// @note: would recommend checking for nullptrs
			I::GameTraceManager->TraceShape(&ray, pLocalPawn.GetEyePosition(), vecPos, &filter, &trace);
			// check if the hit entity is the one we wanted to check and if the trace end point is visible
			if ((uintptr_t)trace.m_pHitEntity != pPawn || !trace.IsVisible())// if invisible, skip this entity
				continue;

			// Get the distance/weight of the move 
			float flCurrentDistance = GetAngularDistance(pBaseCmd, vecPos, &pLocalPawn);

			// Better move found, override.
			auto CurTimePoint = std::chrono::steady_clock::now();


			if (flCurrentDistance < flDistance)
			{
				if (CurTimePoint - LastTimePoint >= std::chrono::milliseconds(500))
				{

					BestTarget = playPawn;
					flDistance = flCurrentDistance;
					vecBestPosition = vecPos;
					LastTimePoint = CurTimePoint;

				}
			}



		}
	}


	

	// Check if a target was found
	if (BestTarget.pthis == NULL )
	{
	
		return;
	}
		

	// Point at them
	QAngle_t* pViewAngles = &(pBaseCmd->pViewAngles->angValue); // Just for readability sake!
	QAngle_t tempViewAngles = *pViewAngles;
	BestTarget.get_bone_data(6, vecBestPosition);

	// Find the change in angles
	QAngle_t vNewAngles = GetAngularDifference(pBaseCmd, vecBestPosition, &pLocalPawn);

	
	// Get the smoothing
	
	auto aimPunch = GetRecoil(pBaseCmd, &pLocalPawn); //get AimPunch angles
	// Apply smoothing and set angles
	//pViewAngles->x += (vNewAngles.x - aimPunch.x) ;// minus AimPunch angle to counteract recoil
	//pViewAngles->y += (vNewAngles.y - aimPunch.y) ;
	//pViewAngles->Normalize();
	
	 
	
	//// Get the class info
	//void* pClassInfo = nullptr;
	//((CEntityInstance*)trace.m_pHitEntity)->GetSchemaClassInfo(&pClassInfo);
	//if (pClassInfo == nullptr)
	//	return;
	//char* pszName = *reinterpret_cast<char**>((uintptr_t)pClassInfo + 0x8);
	//const FNV1A_t uHashedName = FNV1A::Hash(pszName);

	//// Make sure they're a player controller
	//if (uHashedName != FNV1A::HashConst("CCSPlayerController"))
	//	return;


	/*if (!CCSPlayerController((uintptr_t)trace.m_pHitEntity).GetIsAlive() || CCSPlayerController((uintptr_t)trace.m_pHitEntity).GetTeamID() == pLocalController->GetTeamID())
		return;*/

	//pCmd->nButtons.nValue |= IN_ATTACK;

	
	tempViewAngles.x += (vNewAngles.x - aimPunch.x) ;// minus AimPunch angle to counteract recoil
	tempViewAngles.y += (vNewAngles.y - aimPunch.y) ;
	tempViewAngles.Normalize();

	SetViewAngle(tempViewAngles, *pViewAngles);

	//slient aim
	//I::Input->GetUserCmd()->SetSubTickAngle({ pViewAngles->x + vNewAngles.x - aimPunch.x , pViewAngles->y + vNewAngles.y - aimPunch.y * 2.f });
	



	

}

void RCS_OnCreateMove(CCSGOInput* pCsgoInput, CUserCmd* pUserCmd,
	QAngle_t* view_angles, CCSPlayerController* pLocalController)
{
	if (!trigger_bot_active)
		return;
	
	C_CSPlayerPawn pLocalPawn((uintptr_t)I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pLocalController->GetPawnHandle()));
	if (pLocalPawn.pthis == NULL)
		return;

	if (!pLocalController->GetIsAlive())
		return;

	
	const auto aimPunch = pLocalPawn.GetAimPunchCache();
	
	QAngle_t correct_angle;
	if (aimPunch.m_size > 0 && aimPunch.m_size < 0xFFFF) {
		correct_angle = aimPunch.m_data[aimPunch.m_size - 1];
	}
	


	const Vector_t dst = pLocalPawn.GetEyePosition() + FromAngle(*view_angles+ correct_angle) * 3000.0f;
	GameTrace_t trace = GameTrace_t();
	TraceFilter_t filter = TraceFilter_t(0x1C3003, &pLocalPawn, nullptr, 4);
	Ray_t ray = Ray_t();
	I::GameTraceManager->TraceShape(&ray, pLocalPawn.GetEyePosition(), dst, &filter, &trace);
	if (!trace.m_pHitEntity)
		return;

	

	void* pEntity = trace.m_pHitEntity;
	void* pClassInfo = nullptr;
	((CEntityInstance*)pEntity)->GetSchemaClassInfo(&pClassInfo);
	if (pClassInfo == nullptr)
		return;
	char* pszName = *reinterpret_cast<char**>((uintptr_t)pClassInfo + 0x8);
	const FNV1A_t uHashedName = FNV1A::Hash(pszName);

	// Make sure they're a player controller
	if (uHashedName != FNV1A::HashConst("C_CSPlayerPawn"))
		return;


	C_CSPlayerPawn pPawn((uintptr_t)pEntity);
	if (pPawn.GetTeamId() == pLocalPawn.GetTeamId())
		return;

	//CCSPlayerController pPlayer((uintptr_t)pEntity);
	static std::chrono::time_point LastTimePoint = std::chrono::steady_clock::now();
	auto CurTimePoint = std::chrono::steady_clock::now();
	if (CurTimePoint - LastTimePoint >= std::chrono::milliseconds(500))
	{
		const bool isAlreadyShooting = GetAsyncKeyState(VK_LBUTTON) < 0;
		if (!isAlreadyShooting)
		{
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		}

		LastTimePoint = CurTimePoint;
	}

}


