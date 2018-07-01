#ifndef DDRW_RECTANGLE_H_INCLUDED
#define DDRW_RECTANGLE_H_INCLUDED

#include <d3d9.h>
#include <d3dx9.h>
#include <stdbool.h>
#include "D9DW_Config.h"


class D9DW_Rectangle
{
private:

    ID3DXLine* gLine;
    IDirect3DDevice9* pDev;

    inline void clearoutArea(int x, int y, int width, int height, UINT32 rgb_alpha);

public:
    HRESULT Create(IDirect3DDevice9* pDevice)
    {
        pDev = pDevice;
        return D3DXCreateLine(pDev, &gLine);
    }

    void Release(void)
    {
        gLine->Release();
    }

    void Draw(int x , int y, int width, int height, UINT32 rgb_alpha, bool doFill);

};

#endif // DDRW_RECTANGLE_H_INCLUDED
