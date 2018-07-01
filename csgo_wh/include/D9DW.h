#ifndef DDRW_HPP_INCLUDED
#define DDRW_HPP_INCLUDED

#include <d3d9.h>
#include <d3dx9.h>
#include <d3d9types.h>
#include <stdbool.h>

#include "Game.h"
#include "D9DW_Config.h"
#include "D9DW_Text.h"
#include "D9DW_Rectangle.h"


#define MENU_DISABLE    0x1
#define MENU_ESP        0x2


class D9DW
{

private:
    IDirect3DDevice9* pDevice;

    void drawESP(UINT32 idx, ENTITY *ent, FLOAT pos[3]);

    void doESP(Game *game);

    void __stdcall doMenu(Game *game, int startx, int starty, int heightpad);

public:
    D9DW_Text pTxt;
    D9DW_Rectangle pRec;

    void Create(IDirect3DDevice9 *pDev);

    void Release(void);

    void __stdcall Render(Game* game, bool bActive);

    HRESULT checkDxDevice(void);

};

#endif // DDRW_HPP_INCLUDED
