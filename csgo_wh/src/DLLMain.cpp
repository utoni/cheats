#include "DLLMain.h"

#include <cstdio>
#include <d3d9.h>
#include <stdarg.h>
#include <windows.h>
#include <psapi.h>
#include <commctrl.h>

#include "Hook.h"
#include "D9DW.h"
#include "Game.h"

#define DXD9DEVICE_OFFSET 0x198298 + 0x44


typedef __int32 (__stdcall* EndScene_t)(LPDIRECT3DDEVICE9);

static LPDIRECT3DDEVICE9 pDevice = NULL;
static bool bActive = true;
static EndScene_t pEndScene = NULL;
static D9DW ddraw;
//static Game game;


__int32 __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice_t)
{
    asm volatile ("nop; nop; nop");
    int retOrigEndScene = D3D_OK;
    if(pDevice == NULL)
    {
        pDevice = pDevice_t;
        //DEBUG_LOG("D3D9 Device (%X): %p", (UINT32)((UINT32) game.m_shaderapi.lpBaseOfDll + DXD9DEVICE_OFFSET), pDevice);
        DEBUG_LOG("EndScene Arg: %p", pDevice_t);
        DEBUG_FLUSH;
        ddraw.Create(pDevice);
    }
    else
    {
        HRESULT c_ret = ddraw.checkDxDevice();
        if (c_ret == D3D_OK)
        {
            //game.ReadCVars();
            ddraw.Render(/* &game */ NULL, /* game.ReadEntities() */ false);
        }
        else pEndScene = NULL;
        retOrigEndScene = (pEndScene != NULL ? pEndScene(pDevice_t) : D3D_OK);
    }
    asm volatile ("nop; nop; nop");
    return retOrigEndScene;
}

DWORD WINAPI MainThread(void *arg)
{
    //game.Init();
    Hook::hookEndScene(hkEndScene, &pEndScene, false);
    while ( bActive )
    {
        Sleep(500);
        if ( GetForegroundWindow() != FindWindow( 0, "Counter-Strike: Global Offensive" ) )
        {
            pDevice = NULL;
            ddraw.Release();
        }
    }
    return 0;
}

__declspec(dllexport) BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DWORD dwThreadId;
        DisableThreadLibraryCalls(hinstDLL);
        CreateThread(NULL, 0, MainThread, NULL, 0, &dwThreadId);
    }
    return TRUE; // succesful
}
