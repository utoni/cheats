#ifndef D9DW_TEXT_H_INCLUDED
#define D9DW_TEXT_H_INCLUDED

#include <d3d9.h>
#include <d3dx9.h>
#include <stdbool.h>


class D9DW_Text
{

private:
    static bool bInit;
    static ID3DXFont* m_pFont;

public:
    bool isInitialized(void) {
        return this->bInit;
    }

    void Create(IDirect3DDevice9* pDev);

    void Release(void);

    void DrawText(int x, int y, UINT32 rgb_alpha, const char *s_text, va_list p_va);

    void DrawText(int x, int y, UINT32 rgb_alpha, const char *s_text, ...)
    {
        va_list va;
        va_start(va, s_text);
        this->DrawText(x, y, rgb_alpha, s_text, va);
        va_end(va);
    }
};

#endif // D9DW_TEXT_H_INCLUDED
