#include "DLLMain.h"

#include <cstdio>
#include <d3d9.h>
#include <d3dx9.h>

#include "D9DW_Text.h"

bool D9DW_Text::bInit = false;
ID3DXFont* D9DW_Text::m_pFont = NULL;


void D9DW_Text::Create(IDirect3DDevice9* pDev)
{
    if (!bInit)
    {
        D3DXCreateFont(pDev, 15, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &m_pFont);
        bInit = true;
    }
}

void D9DW_Text::Release(void)
{
    if (bInit)
    {
        bInit = false;
        m_pFont->Release();
        m_pFont = NULL;
    }
}

void D9DW_Text::DrawText(int x, int y, UINT32 rgb_alpha, const char *s_text, va_list p_va)
{
    if (!bInit) return;
    RECT rct;
    rct.left=x;
    rct.top=y;
    rct.right=rct.left+350;
    rct.bottom=rct.top+350;
    char logbuf[100] = {0};
    memset(&logbuf[0], '\0', 100);
    _vsnprintf(logbuf, sizeof(logbuf), s_text, p_va);
    m_pFont->DrawTextA(NULL, logbuf, sizeof(logbuf), &rct, 0, rgb_alpha);
}
