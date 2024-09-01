#pragma once

#include "common.h"

class CUserCmd;
class CBaseUserCmdPB;
class CCSPlayerController;

namespace F
{


	
	void OnCreateMove(CUserCmd* pCmd, CBaseUserCmdPB* pBaseCmd, CCSPlayerController* pLocalController);
	
}
