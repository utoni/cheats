#include "DLLMain.h"
#include "D9DW_Rectangle.h"

#include <cstdio>
#include <d3d9.h>
#include <d3dx9.h>


inline void D9DW_Rectangle::clearoutArea(int x, int y, int width, int height, UINT32 rgb_alpha)
{
    D3DRECT rect;
    rect.x1 = x;
    rect.x2 = x + width;
    rect.y1 = y;
    rect.y2 = y + height;
    this->pDev->Clear(1, &rect, D3DCLEAR_TARGET, rgb_alpha, 0.0f, 0);
}

void D9DW_Rectangle::Draw(int x , int y, int width, int height, UINT32 rgb_alpha, bool doFill)
{
    D3DXVECTOR2 points[8];
    points[0] = D3DXVECTOR2(x, y);
    points[1] = D3DXVECTOR2(x + width, y);
    points[2] = D3DXVECTOR2(x + width, y);
    points[3] = D3DXVECTOR2(x + width, y + height);
    points[4] = D3DXVECTOR2(x + width, y + height);
    points[5] = D3DXVECTOR2(x, y + height);
    points[6] = D3DXVECTOR2(x, y + height);
    points[7] = D3DXVECTOR2(x, y);
    this->gLine->SetPattern(0xffffffff);
    this->gLine->SetPatternScale(2.0f);
    this->gLine->Begin();
    this->gLine->Draw(points, 8, rgb_alpha);
    this->gLine->End();
    D9DW_Rectangle::clearoutArea(x+3, y+3, width-6, height-6, 0x770077AA);
}
