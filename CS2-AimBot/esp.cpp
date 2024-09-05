#include <cstdint>
#include <Windows.h>
#include "client.dll.hpp"
#include "offset.hpp"
#include "render.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <corecrt_math_defines.h>
#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include <cmath>
#include "interfaces.h"
#include "hooks.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

#include <d3d11.h>
#include "cgameresourceservice.h"
#include "cgameentitysystem.h"
#include "fnv1a.h"
#include "entity.h"
#include "esp.h"
#include "iengineclient.h"
#include "D3D11/MyD3d11.h"

extern MyD3D11 g_myD3d11;
extern  std::uintptr_t client_address;
using namespace cs2_dumper::offsets::client_dll;
extern HWND hWindow;



extern int screenWidth;
extern int screenHeight;





void do_esp()
{

    if (!I::Engine->IsConnected() || !I::Engine->IsInGame())
        return;

    const auto& m_vOldOrigin = cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin;

    // client.dll.hpp

    const auto& m_iTeamNum = cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum; // uint8
    const auto& m_lifeState = cs2_dumper::schemas::client_dll::C_BaseEntity::m_lifeState; // uint8
    const auto& m_hPlayerPawn = cs2_dumper::schemas::client_dll::CCSPlayerController::m_hPlayerPawn; // CHandle<C_CSPlayerPawn>
    const auto& m_vecViewOffset = cs2_dumper::schemas::client_dll::C_BaseModelEntity::m_vecViewOffset;  // CNetworkViewOffsetVector
    constexpr std::ptrdiff_t m_iHealth = cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth;; // int32


    uintptr_t localPlayer = *(uintptr_t*)(client_address + dwLocalPlayerPawn);

    if (!localPlayer)return;

    Vector_t Localorgin = *(Vector_t*)(localPlayer + m_vOldOrigin);
    view_matrix_t view_matrix = *(view_matrix_t*)(client_address + dwViewMatrix);
    uintptr_t entity_list = *(uintptr_t*)(client_address + dwEntityList);

    if (!entity_list)return;

    int localTeam = *reinterpret_cast<int*>(localPlayer + m_iTeamNum);
    // RGB enemy
    static int r = 255;
    static int g = 0;
    static int b = 255;

    // RGB team
    static int t_r = 0;
    static int t_g = 160;
    static int t_b = 255;

   
    D3DCOLORVALUE enemy_color = { t_r, t_g, t_b,1.0f};
  

    const int iHighestIndex = I::GameResourceService->pGameEntitySystem->GetHighestEntityIndex();
 
    uintptr_t LocalControllerAddress = (uintptr_t)I::GameResourceService->pGameEntitySystem->Get<CCSPlayerController>(I::Engine->GetLocalPlayer());
    if (LocalControllerAddress == NULL)return;
    CCSPlayerController localplayer(LocalControllerAddress);
    int ourteam = localplayer.GetTeamID();

    for (int playerIndex = 1; playerIndex < iHighestIndex; ++playerIndex) {

        // Get the entity
        uintptr_t pEntity = (uintptr_t)I::GameResourceService->pGameEntitySystem->Get(playerIndex);
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


        CCSPlayerController pPlayer(pEntity);

      

        int playerTeam = pPlayer.GetTeamID();

       

        //// Check the entity is not us
        if (pPlayer.Address == LocalControllerAddress)
            continue;

        if (ourteam == playerTeam)
            continue; 

        // Get the player pawn
        uintptr_t pPawn = (uintptr_t)I::GameResourceService->pGameEntitySystem->Get<C_CSPlayerPawn>(pPlayer.GetPawnHandle());


        C_CSPlayerPawn playPawn(pPawn);
        if (pPawn == NULL)
            continue;

        uintptr_t pGameSceneNode = *reinterpret_cast<uintptr_t*>(pPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
        if (!pGameSceneNode)continue;
        if (*reinterpret_cast<bool*>(pGameSceneNode + cs2_dumper::schemas::client_dll::CGameSceneNode::m_bDormant))continue;


        int health = playPawn.GetHealth();

        if (health <= 0 || health > 100)
            continue;

    

        

        Vector_t orgian = (Vector_t)playPawn.GetPos();
        Vector_t head = { orgian.x, orgian.y, orgian.z + 75.f };

        Vector_t screenPos = orgian.WTS(view_matrix);
        Vector_t screenHead = head.WTS(view_matrix);

        float height = screenPos.y - screenHead.y;
        float width = height / 2.4f;

        int rectBottomX = screenHead.x;
        int rectBottomY = screenHead.y + height;

        int bottomCenterX = screenWidth / 2;
        int bottomCenterY = screenHeight;

        if (screenHead.x - width / 2 >= 0 &&
            screenHead.x + width / 2 <= screenWidth &&
            screenHead.y >= 0 &&
            screenHead.y + height <= screenHeight &&
            screenHead.z > 0) {

    


                g_myD3d11.DrawLine(bottomCenterX, bottomCenterY, rectBottomX, rectBottomY, enemy_color);
                g_myD3d11.DrawBox(screenHead.x - width / 2,
                    screenHead.y,
                    width,
                    height,
                    enemy_color);


           
        }
    }

}
void do_esp_destroy()
{
   
    
    
}

void do_present() {
    
        


  

     do_esp();



   




}