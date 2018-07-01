#ifndef AOE2HD_H_INCLUDED
#define AOE2HD_H_INCLUDED

#define DUMMY5 0x90,0x90,0x90,0x90,0x90      /* nop; nop; nop; nop; nop */

/* SAFE! */
#define MAP_NOFOG  0x45BE43
#define MAP_NOFOG0 0x8B,0x0C,0x81                   /* mov ecx,[ecx+eax*4] */
#define MAP_NOFOG1 0x8B,0x45,0x10                   /* mov eax,[ebp+10] */
#define MAP_NOFOGI 0x81,0xC9,0x00,0x04,0x00,0x00    /* or ecx,0x00000400 */

/* SAFE! */
#define MAP_MINI  0x46CA33
#define MAP_MINI0 0x8B,0x0C,0x88                /* mov ecx,[eax+ecx*4] */
#define MAP_MINI1 0x8B,0x87,0x34,0x01,0x00,0x00 /* mov eax,[edi+00000134] */
#define MAP_MINII 0x81,0xC9,0x00,0x00,0x00,0x04 /* or ecx,0x04000000 */

/* NOT SAFE -> DESYNC POSSIBLE! */
#define MAP_SMTH  0x46CEE8
#define MAP_SMTH0 0x8B,0x04,0x88                /* mov eax,[eax+ecx*4] */
#define MAP_SMTH1 0x8B,0x8F,0x34,0x01,0x00,0x00 /* mov ecx,[edi+00000134] */
#define MAP_SMTHI 0x0D,0x00,0x04,0x00,0x00      /* or eax,0x00000400 */

/* NOT SAFE! .> DESYNC POSSIBLE! */
#define MAP_UNIT  0x47F851
#define MAP_UNIT0 0x8B,0x01                               /* mov eax,[ecx] */
#define MAP_UNIT1 0x8B,0xD0,0x8B,0x8D,0x34,0xFF,0xFF,0xFF /* mov edx,eax; mov ecx,[ebp-000000CC] */
#define MAP_UNITI 0x0D,0x00,0x04,0x00,0x00                /* or eax,0x00000400 */

/* MAP/MINIMAP FLAGS:
 *  NOFOG_BY_UNIT.....: 0x00000002
 *  NOFOG_ALL.........: 0x00000400
 *  DISCOVERED_BY_UNIT: 0x00020000
 *  DISCOVERED_ALL....: 0x04000000
 *  MAP_FULL_VISIABLE.: DISCOVERED_ALL | NOFOG_ALL
 *  MAP_SPY_LIKE......: DISCOVERED_BY_UNIT | NOFOG_BY_UNIT
 */

struct resources
{
    float food;
    float wood;
    float stone;
    float gold;
    float remainingPop;
    unsigned char garbage_1[4];
    float currentAge;
    unsigned char garbage_2[16];
    float currentPop;
};

struct mapsize
{
    uint32_t cells_x;
    uint32_t cells_y;
};

#endif // AOE2HD_H_INCLUDED
