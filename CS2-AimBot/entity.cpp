#include "entity.h"
#include "client.dll.hpp"



std::uint32_t C_CSPlayerPawn::GetOwnerHandleIndex()
{
	std::uint32_t Result = -1;
	if (pthis && GetCollision() && !(GetCollision()->GetSolidFlags() & 4))
		Result = this->GetOwnerHandle().GetEntryIndex();

	return Result;
}


std::uint16_t C_CSPlayerPawn::GetCollisionMask()
{
	if (pthis && GetCollision())
		return GetCollision()->CollisionMask(); // Collision + 0x38

	return 0;
}



int CCSPlayerController::GetTeamID()
{
	using namespace cs2_dumper::schemas::client_dll;
	
	return *reinterpret_cast<int*>(Address+ C_BaseEntity::m_iTeamNum);
}

int CCSPlayerController::GetHealth()
{
	using namespace cs2_dumper::schemas::client_dll;
	return *reinterpret_cast<int*>(Address + C_BaseEntity::m_iHealth);
}

int CCSPlayerController::GetIsAlive()
{
	
	return *reinterpret_cast<int*>(Address + cs2_dumper::schemas::client_dll::CCSPlayerController::m_bPawnIsAlive);
}