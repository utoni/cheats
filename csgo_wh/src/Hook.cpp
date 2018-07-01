#include <cstdio>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdarg.h>
#include <windows.h>
#include <psapi.h>
#include <commctrl.h>

#include "Hook.h"
#include "Game.h"


bool Hook::hookEndScene(EndScene_t pHookFunc, EndScene_t *pEndScene, bool unhook)
{
    HMODULE hm_ddrw = GetModuleHandleA("d3d9.dll");
    MODULEINFO md_ddrw;
    LPVOID p_fEndScene;


    memset(&md_ddrw, '\0', sizeof(md_ddrw));
    if (hm_ddrw != NULL && GetModuleInformation(GetCurrentProcess(), hm_ddrw, &md_ddrw, sizeof(md_ddrw)) == TRUE)
    {
        DEBUG_LOG("MODINFO(d3d9.dll): 0x%p (%lu)", md_ddrw.lpBaseOfDll, md_ddrw.SizeOfImage);
        if ( (p_fEndScene = (LPVOID)( (PBYTE)md_ddrw.lpBaseOfDll + ENDSCENE_OFFSET )) != NULL )
        {
            DEBUG_LOG("ENDSCENE: 0x%p | HOOK: 0x%p | REL_OFF: %lu", p_fEndScene, pHookFunc, (DWORD)( (DWORD)pHookFunc - (DWORD)p_fEndScene));
            if (!unhook)
            {
                *pEndScene = (EndScene_t) Hook::Detour((PBYTE)p_fEndScene,(PBYTE)pHookFunc);
            }
            else
            {
                Hook::UnDetour((PBYTE)p_fEndScene);
            }
        }
        else return false;
    }
    else return false;
    return true;
}

BYTE* Hook::Detour(BYTE *src, const BYTE *dst)
{
    BYTE *jmp = (BYTE *) calloc(1, 0x5);
    DWORD dwback;
    VirtualProtect(src - 0x5, 0x7, PAGE_READWRITE, &dwback); // important for changing opcodes in the code section
    jmp[0] = 0xE9; // far jump (32bit offset signed)
    *(DWORD*)(jmp+1) = (DWORD)(dst - src);
    memcpy(src - 0x5, jmp, 0x5);
    src[0] = 0xEB; // short jump (8bit offset signed)
    src[1] = 0xF9; // two complement -> -0x7
    VirtualProtect(src - 0x5, 0x7, dwback, &dwback);
    free(jmp);
    return (src + 0x2); // return the REAL function addr -> MOV EDI,EDI = 2 bytes
}

void Hook::UnDetour(BYTE *src)
{
    BYTE *jmp = (BYTE *) calloc(1, 0x5);
    DWORD dwback;
    VirtualProtect(src - 0x5, 0x7, PAGE_READWRITE, &dwback);
    src[0] = 0x8B; // MOV opcode
    src[1] = 0xFF; // operand0: EDI, operand1: EDI
    jmp[0] = 0x90; // overwrite JMP with NOP-sled
    jmp[1] = 0x90;
    jmp[2] = 0x90;
    jmp[3] = 0x90;
    jmp[4] = 0x90;
    memcpy(src - 0x5, jmp, 0x5);
    VirtualProtect(src - 0x5, 0x7, dwback, &dwback);
    free(jmp);
}
