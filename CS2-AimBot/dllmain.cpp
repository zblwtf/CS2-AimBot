// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <windows.h>
#include "interfaces.h"
#include "hooks.h"
#include <stdio.h>
#include "offset.hpp"
#include "tool.h"
#include "client.dll.hpp"
HWND hWindow;
window_size window_info;
FILE* stream;
uintptr_t client_address;
extern bool trigger_bot_active;
static BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    const auto MainWindow = [handle]()
        {
            return GetWindow(handle, GW_OWNER) == nullptr &&
                IsWindowVisible(handle) && handle != GetConsoleWindow();
        };

    DWORD nPID = 0;
    GetWindowThreadProcessId(handle, &nPID);

    if (GetCurrentProcessId() != nPID || !MainWindow())
        return TRUE;

    *reinterpret_cast<HWND*>(lParam) = handle;
    return FALSE;
}
HWND Get_Window_Handle()
{ 

    HWND hWin = nullptr;
    hWin =  FindWindowA("SDL_app", "Counter-Strike 2");
    /*while (hWin == nullptr)
    {
        EnumWindows(::EnumWindowsCallback, reinterpret_cast<LPARAM>(&hWin));
        ::Sleep(200U);
    }*/
    return hWin;

}
bool Setup()
{
    client_address = (uintptr_t)GetModuleHandleA("client.dll");
   hWindow = Get_Window_Handle();
   window_info = get_window_size();
   AllocConsole();
   freopen_s(&stream, "CONOUT$", "w", stdout);

    if (!I::Setup())
    {
        return false;
    }
    if (!H::Setup())
    {
        return false;
    }


}

DWORD WINAPI start_point(HMODULE hModule)
{


    Setup(); 
    Vector_t oPunch{ 0,0,0 };
    while (!GetAsyncKeyState(VK_END))
    {
        if (GetAsyncKeyState(VK_F2) & 0x8000)
        {
            trigger_bot_active = true;
            if (trigger_bot_active)
            {
                printf("[trigger_bot_status] : Active\n");
            }
        }
        if (GetAsyncKeyState(VK_F3) & 0x8000)
        {
            trigger_bot_active = false;
            if (!trigger_bot_active)
            {
                printf("[trigger_bot_status] : !Active\n");
            }
        }
        Sleep(10);
        
    }

    freopen_s(&stream, "CONOUT$", "w", stdout);

    CloseHandle(stream);
    // free allocated memory for console
    if (::FreeConsole() != TRUE)
        return 0;

    // close console window
    if (const HWND hConsoleWindow = ::GetConsoleWindow(); hConsoleWindow != nullptr)
        ::PostMessageW(hConsoleWindow, WM_CLOSE, 0U, 0L);

    H::Destroy();


    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)start_point, hModule, NULL, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

