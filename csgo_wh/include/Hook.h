#ifndef HOOK_H_INCLUDED
#define HOOK_H_INCLUDED

#define ENDSCENE_OFFSET 0x2179F

typedef __int32 (__stdcall* EndScene_t)(LPDIRECT3DDEVICE9);

class Hook
{
public:

    static bool hookEndScene(EndScene_t pHookFunc, EndScene_t *pEndScene, bool unhook);

    static BYTE* Detour(BYTE *src, const BYTE *dst);

    static void UnDetour(BYTE *src);

};

#endif // HOOK_H_INCLUDED
