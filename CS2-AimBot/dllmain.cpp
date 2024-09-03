// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include <windows.h>
#include "interfaces.h"
#include "hooks.h"
#include <stdio.h>
#include "offset.hpp"
#include "tool.h"
#include "client.dll.hpp"
#include "esp.h"


HWND hWindow;
FILE* stream;
uintptr_t client_address;
uintptr_t engine2_address;
extern bool trigger_bot_active;

int screenWidth = 1024;
int screenHeight = 726;



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
    engine2_address = (uintptr_t)GetModuleHandleA("engine2.dll");

    screenWidth = *(int*)(engine2_address + cs2_dumper::offsets::engine2_dll::dwWindowWidth);
    screenHeight = *(int*)(engine2_address + cs2_dumper::offsets::engine2_dll::dwWindowHeight);


   hWindow = Get_Window_Handle();
 
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

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Increase sleep duration
      
        
    }

    if (stream)
        CloseHandle(stream);

    // free allocated memory for console
    if (::FreeConsole() != TRUE)
        return 0;

    // close console window
    if (const HWND hConsoleWindow = ::GetConsoleWindow(); hConsoleWindow != nullptr)
        ::PostMessageW(hConsoleWindow, WM_CLOSE, 0U, 0L);

    H::Destroy();
    do_esp_destroy();

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

