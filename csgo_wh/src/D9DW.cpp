#include "DLLMain.h"

#include <cstdio>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdarg.h>
#include <windows.h>
#include <psapi.h>
#include <commctrl.h>

#include "D9DW.h"

#define MENUCOLOR_DEFAULT  D3DCOLOR_ARGB(0xAA, 0x77, 0x00, 0x77)
#define MENUCOLOR_ACTIVE   D3DCOLOR_ARGB(0xAA, 0x11, 0xAA, 0x00)
#define MENUCOLOR_INACTIVE D3DCOLOR_ARGB(0xAA, 0xAA, 0x11, 0x00)


typedef void (__stdcall *keyPressed_t)(D9DW *, Game *, bool);
struct st_menuEntry
{
    bool active;
    char *name;
    int id;
    keyPressed_t callback;
};
typedef struct st_menuEntry st_menuEntry;


void __stdcall showDebugCB(D9DW *cPtr, Game *game, bool active)
{
    UINT32 i;
    cPtr->pTxt.DrawText(450, 1 , MENUCOLOR_DEFAULT, "%u x %u / %u", game->dwWidth, game->dwHeight, game->dwFov);
    for (i = 0; i < game->dwPlayerCount; i++)
    {
        cPtr->pTxt.DrawText(450, 20+(i*15), MENUCOLOR_DEFAULT, "[%u] [%8.2f,%8.2f,%8.2f]", i, game->g_entities[i].p_pos[0], game->g_entities[i].p_pos[1], game->g_entities[i].p_pos[2]);
    }
}


static st_menuEntry menuEntries[] = { { false, (char*) "toggle esp", VK_F2, NULL }, { false, (char*) "toggle debug", VK_F3, showDebugCB } };
static const int bMenuKey = VK_F1;
static bool bMenu = false;


void __stdcall D9DW::doMenu(Game *game, int startx, int starty, int heightpad)
{
    size_t idx;

    if (GetKeyState(bMenuKey) &1)
    {
        bMenu = !bMenu;
    }
    if (bMenu)
    {
        this->pTxt.DrawText(startx, starty, MENUCOLOR_INACTIVE, "disable menu");
    }
    for (idx = 0; idx < sizeof(menuEntries)/sizeof(menuEntries[0]); idx++)
    {
        if (GetKeyState(menuEntries[idx].id) &1)
        {
            menuEntries[idx].active = !menuEntries[idx].active;
            if (menuEntries[idx].callback) menuEntries[idx].callback(this, game, menuEntries[idx].active);
        }
        if (bMenu)
        {
            DEBUG_LOG("%u: %s", idx, menuEntries[idx].name);
            this->pTxt.DrawText(startx, starty + ((idx+1)*heightpad), (menuEntries[idx].active ? MENUCOLOR_ACTIVE : MENUCOLOR_INACTIVE), "[%d] - %s", menuEntries[idx].id, menuEntries[idx].name);
        }
    }
}

void __stdcall D9DW::Render(Game* game, bool bActive)
{
    if (bActive)
    {
        //this->doMenu(game, 5, 300, 12);
        this->pTxt.DrawText(2, 1 , MENUCOLOR_DEFAULT, "%s %s - [F1 MENU] - %d Player", COPYRIGHT, VERSION, game->dwPlayerCount);
    }
    else
    {
        this->pTxt.DrawText(2, 1 , MENUCOLOR_DEFAULT, "%s %s - WAITING FOR GAME ...", COPYRIGHT, VERSION, game->dwPlayerCount);
    }
}

void D9DW::Create(IDirect3DDevice9 *pDev)
{
    this->pDevice = pDev;
    this->pTxt.Create(pDev);
    this->pRec.Create(pDev);
}

void D9DW::Release(void)
{
    this->pTxt.Release();
    this->pRec.Release();
}

HRESULT D9DW::checkDxDevice(void)
{
    return ( D9DW::pDevice->TestCooperativeLevel() );
}

void D9DW::drawESP(UINT32 idx, ENTITY *ent, FLOAT pos[3])
{
}

void D9DW::doESP(Game *game)
{
    UINT32 i;
    FLOAT pos[3];

    for (i = 0; i < game->dwPlayerCount; i++)
    {
        pos[0] = game->g_entities[i].p_pos[0] - game->g_localPlayer->p_pos[0];
        pos[1] = game->g_entities[i].p_pos[1] - game->g_localPlayer->p_pos[1];;
        pos[2] = game->g_entities[i].p_pos[2] - game->g_localPlayer->p_pos[2];;
        this->drawESP(i, &game->g_entities[i], pos);
    }
}
