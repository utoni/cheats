/*****************************************
 * Simple H1Z1 Radar/MemDumper
 *
 * Mayb DETECTED, RPM could be hook'd to detect such things!
 * Read it, but DONT use it!
 *
 * coded by lnslbrty/dev0, \x90
 *
 * recommendation (mingw64):
 *    CFLAGS=-march=athlon64 -Wall -g -DPSAPI_VERSION=1 -DUNICODE=1 -D_UNICODE=1 -g
 *    LDFLAGS=-static-libgcc -static [PATH_TO_LIBPSAPI].a [PATH_TO_LIBKERNEL32].a
 *    Windows PowerShell as Terminal Emulator
 *****************************************/

#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <ghack.h>

#define BPATTERN_SIZ(pat) (sizeof(pat)/sizeof(pat[0]))
#define GAME_PATTERN { 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xD0, 0xD2 }
#define GAME_MASK    "x???????????????xx"
#define GAME_OFFSET  0x8

#define ENTNEXT 0x410
#define ENTCOUNT 0x688
#define ENTFIRST 0x11D8

#define O_CARPOS   0x0250
#define O_PLYSTT   0x0028
#define O_NAME     0x04E8
#define O_STANCE   0x05C8
#define O_YAW      0x0240
#define O_TYPE     0x05D0
#define O_ALIVE    0x139C
#define O_POSOFF   0x0198
#define O_POS      0x0100
#define O_VWANGLX  0x02E0
#define O_VWANGLY  O_VWANGLX+0x4

#define MIN_VEC_VAL -10000.0f
#define MAX_VEC_VAL 10000.0f

#define TYPELEN 32
#define NAMELEN 32
#define PROCLEN 256

#ifdef _ENABLE_HRADAR
#define HIZI_MAXDIST 260.0f
#define ENABLE_HRADAR 1
#endif // ENABLE_HRADAR

//#define readMem(baseAdr, data, siz) _readMem(baseAdr, data, siz, __FILE__, __LINE__)
#ifdef ENABLE_HRADAR
#define ENT(id, nm, col) { id, nm, col }
static struct radarInfo *rdr_entCnt = NULL;
#else
#define ENT(id, nm, col) { id, nm }
#endif

typedef UINT64 QWORD;

HANDLE hndl_proc;
static LPCVOID adr_game = NULL;
static LPCVOID adr_base = NULL;
static LPCVOID adr_entityCount = NULL;
static LPCVOID adr_firstEntity = NULL;
static UINT32 entityCount = 0;

struct e_type
{
    DWORD id;
    const CHAR name[TYPELEN];
#ifdef ENABLE_HRADAR
    enum radarColor color;
#endif // ENABLE_HRADAR
};
#define ETYPE_PLAYERM 0x04
#define ETYPE_PLAYERF 0x05
#define ETYPE_ZOMBIE  0x0C
#define ETYPE_ZOMBIE2 0x5b
#define ETYPE_DEER    0x13
#define ETYPE_BEAR    0x50
#define ETYPE_WOLF    0x14
#define ETYPE_RABBIT  0x55
#define ETYPE_WEAPON  0x34
#define ETYPE_LOOT    0x2E
#define ETYPE_OFFROAD 0x11
#define ETYPE_PICKUP  0x72
#define ETYPE_POLICE  0x76


struct e_type t_entities[] =
{
    ENT(ETYPE_PLAYERM, "PLAYER_MAL", RC_RED), ENT(ETYPE_PLAYERF, "PLAYER_FEM", RC_RED), ENT(ETYPE_ZOMBIE, "ZOMBIE", RC_GREEN), ENT(ETYPE_ZOMBIE2, "ZOMBIE", RC_GREEN), ENT(ETYPE_BEAR, "BEAR", RC_GREEN), ENT(ETYPE_WOLF, "WOLF", RC_GREEN),
    ENT(0x15, "AMMO", RC_MAGENTA), ENT(ETYPE_OFFROAD, "OffRoad", RC_MAGENTA), ENT(ETYPE_PICKUP, "Pickup", RC_MAGENTA), ENT(ETYPE_POLICE, "PoliceCar", RC_MAGENTA),
    ENT(0x2C, "USABLE", RC_BLUE),
    ENT(0x9C, "LANDMINE", RC_CYAN), ENT(0x33, "STORAGE", RC_YELLOW), ENT(0x6D, "LOOTSTASH", RC_YELLOW),
    ENT(0x6F, "LOCKER", RC_WHITE), ENT(0x70, "LOCKER", RC_WHITE), ENT(ETYPE_DEER, "DEER", RC_WHITE), ENT(0x55, "RABBIT", RC_WHITE),
    ENT(ETYPE_LOOT, "*LOOT*", RC_YELLOW), ENT(ETYPE_WEAPON, "WEAPON", RC_YELLOW)
};
#define t_entities_siz sizeof(t_entities)/sizeof(t_entities[0])

struct g_entity
{
    LPCVOID adr;
    FLOAT pos[3];
    float yaw;
    char name[NAMELEN];
    DWORD stance;
    DWORD type;
    BYTE isAlive;
    struct g_entity *next;
};

static LPCVOID localEntitiy;
static struct g_entity localPlayer;
#define PLAYER_DEFAULT 0x1
#define PLAYER_INCAR   0x2
static BYTE localPlayerStatus = PLAYER_DEFAULT;
static FLOAT localViewAngle[2];


#ifndef ENABLE_HRADAR
static void clrscr(void)
{
    HANDLE                     hStdOut;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD                      count;
    DWORD                      cellCount;
    COORD                      homeCoords = { 0, 0 };

    hStdOut = GetStdHandle( STD_OUTPUT_HANDLE );
    if (hStdOut == INVALID_HANDLE_VALUE) return;
    if (!GetConsoleScreenBufferInfo( hStdOut, &csbi )) return;
    cellCount = csbi.dwSize.X *csbi.dwSize.Y;
    if (!FillConsoleOutputCharacter(
                hStdOut,
                (TCHAR) ' ',
                cellCount,
                homeCoords,
                &count
            )) return;
    if (!FillConsoleOutputAttribute(
                hStdOut,
                csbi.wAttributes,
                cellCount,
                homeCoords,
                &count
            )) return;
    SetConsoleCursorPosition( hStdOut, homeCoords );
}
#endif

static void getEntityCount(void)
{
    if (adr_base == NULL)
    {
        adr_base = //readPtrH(adr_game, 0);
            readPtr(hndl_proc, adr_game, 0);
        if (adr_base != NULL)
        {
            adr_entityCount = calcOffset( adr_base, ENTCOUNT );
            adr_firstEntity = calcOffset( adr_base, 0 );
        }
    }
    else
    {
        if ( readMemH( adr_entityCount, &entityCount, sizeof(entityCount) ) == 0 )
        {
            exit(1);
        }
    }
}

static inline const CHAR *getEntityTypeA(DWORD id)
{
    int i;
    for (i = 0; i < t_entities_siz; i++)
    {
        if (t_entities[i].id == id)
        {
            return t_entities[i].name;
        }
    }
    return "UNKNOWN";
}

static inline BOOL isEntityOk(DWORD id)
{
    int i;
    for (i = 0; i < t_entities_siz; i++)
    {
        if (t_entities[i].id == id)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static BOOL readEntity(LPCVOID adr, struct g_entity *ge)
{
    int i;
    FLOAT tmp3f[3];
    LPCVOID ptr = NULL;
    if (ge == NULL) return FALSE;

    if ( readMemH( calcOffset(adr, O_TYPE), &ge->type, sizeof(ge->type) ) == 0) return FALSE;
    if (isEntityOk(ge->type) == FALSE) return TRUE;
    readMemH( calcOffset(adr, O_STANCE), &ge->stance, sizeof(ge->stance) );
    readMemH( calcOffset(adr, O_ALIVE), &ge->isAlive, sizeof(ge->isAlive) );
    if (ge->isAlive == 0x1)
    {
        if (ge->type == ETYPE_PLAYERM || ge->type == ETYPE_PLAYERF)
        {
            ptr = readPtrH(adr, O_NAME);
            readMemH( ptr, &ge->name[0], sizeof(CHAR)*NAMELEN );
            readMemH( calcOffset(adr, O_YAW), &ge->yaw, sizeof(ge->yaw) );
        }
        if (localEntitiy == NULL) localEntitiy = adr;
        if (ge->type == ETYPE_OFFROAD || ge->type == ETYPE_PICKUP || ge->type == ETYPE_POLICE)
        {
            readMemH( calcOffset(adr, O_CARPOS), &ge->pos[0], sizeof(FLOAT)*3 );
            readMemH( calcOffset(adr, O_YAW), &ge->yaw, sizeof(ge->yaw) );
        }
        else
        {
            ptr = readPtrH(adr, O_POSOFF);
            readMemH( calcOffset(ptr, O_POS), &tmp3f[0], sizeof(tmp3f) );

            if (adr == localEntitiy)
            {
                readMemH( calcOffset(adr, O_PLYSTT), &localPlayerStatus, sizeof(localPlayerStatus) );
            }
            for (i = 0; i <= 2; i++)
            {
                if (tmp3f[i] < MAX_VEC_VAL && tmp3f[i] > MIN_VEC_VAL)
                {
                    ge->pos[i] = tmp3f[i];
                }
            }
        }
    }
    return ( TRUE );
}

static float calcVecDist(float v1[3], float v2[3])
{
    return sqrtf( powf(v1[0] - v2[0], 2.0f) + powf(v1[1] - v2[1], 2.0f) + powf(v1[2] - v2[2], 2.0f) );
}

#ifndef ENABLE_HRADAR
static void printEntity(LPCVOID adr_ptr, struct g_entity *ge)
{
    const char *etype;

    if (adr_ptr == localEntitiy)
    {
        printf("[LOCALPLAYER] status:%03d | vieAngle[x,y]: %3.2f,%3.2f ", localPlayerStatus, localViewAngle[0], localViewAngle[1]);
    }
    if ( strcmp( (etype = getEntityTypeA(ge->type)), "UNKNOWN") != 0 )
    {
        printf("%p [ENTITY]"
               " type:%04lu (%7s) | alive:%02d | pos_01[x,y,z]:%+8.2f,%+8.2f,%+8.2f | vecDist:%+8.2f | stance:%ld | yaw:%+8.2f | name: %s"
               "\n", adr_ptr, ge->type, getEntityTypeA(ge->type), ge->isAlive,
               ge->pos[0], ge->pos[1], ge->pos[2],
               (ge->isAlive == 0x1 ? calcVecDist( ge->pos, localPlayer.pos ) : 0.0f),
               ge->stance, ge->yaw, (ge->type == ETYPE_PLAYERM || ge->type == ETYPE_PLAYERF ? ge->name : "-")
              );
    }
}
#endif

#ifdef ENABLE_HRADAR
static void doCarRadar(struct g_entity *ge)
{
    if ( localPlayerStatus == PLAYER_INCAR && calcVecDist( ge->pos, localPlayer.pos ) < 3.0f )
    {
        switch (ge->type)
        {
        case ETYPE_OFFROAD:
        case ETYPE_PICKUP:
        case ETYPE_POLICE:
            radarSetPlayerPosition(ge->pos, localViewAngle[0]);
        }
    }
}

static void drawEntityOnRadar(LPCVOID adr_ptr, struct g_entity *ge)
{
    int i;
    enum radarColor color;;
    BOOL b_colFound = FALSE;

    for (i = 0; i < t_entities_siz; i++)
    {
        if (t_entities[i].id == ge->type)
        {
            color = t_entities[i].color;
            b_colFound = TRUE;
            break;
        }
    }
    if (b_colFound == FALSE)
    {
        color = RC_WHITE;
    }
    doCarRadar(ge);
    radarUpdateEntity((UINT64)adr_ptr, ge->pos, color, (ge->type == ETYPE_PLAYERM || ge->type == ETYPE_PLAYERF ? TRUE : FALSE), TRUE);
}
#endif

static FLOAT getViewAngleX(void)
{
    FLOAT ret;

    if ( readFloat(calcOffset(localEntitiy, O_VWANGLX), ret) > 0 )
    {
        return ( M_PI + ret ) * -(180/M_PI) + 270;
    }
    else
    {
        return 0.0f;
    }
}

static FLOAT getViewAngleY(void)
{
    FLOAT ret;

    if ( readFloat(calcOffset(localEntitiy, O_VWANGLY), ret) > 0 )
    {
        return ( M_PI + ret ) * -(180/M_PI) + 270;
    }
    else
    {
        return 0.0f;
    }
}

/*
static void doAim(FLOAT target[3], FLOAT local[3], FLOAT localAngle[2])
{
    float deltaX = (local[0]) - (target[0]);
    float deltaY = (local[1]) - (target[1]);
    float orgAngleX = localAngle[0], orgAngleY = localAngle[1];

    if ((target[0]) > (local[0]) && (target[1]) <= (local[1]))
    {
        localAngle[0] = atanf(deltaX/deltaY) * -180.0f / M_PI;
    }

    else if((target[0]) >= (local[0]) && (target[1]) > (local[1]))
    {
        localAngle[0] = atanf(deltaX/deltaY) * -180.0f / M_PI + 180.0f;
    }

    else if((target[0]) < (local[0]) && (target[1]) >= (local[1]))
    {
        localAngle[0] = atanf(deltaX/deltaY) * -180.0f / M_PI - 180.0f;
    }

    else if((target[0]) <= (local[0]) && (target[1]) < (local[1]))
    {
        localAngle[0] = atanf(deltaX/deltaY) * -180.0f / M_PI + 360.0f;
    }

    float deltaZ = (target[2]) - (local[2]);
    float dist = sqrt(
                     powf( (target[0] - local[0]), 2.0) +
                     powf( (target[1] - local[1]), 2.0) +
                     powf( (target[2] - local[2]), 2.0)
                 );

    localAngle[1] = asinf(deltaZ/dist) * 180.0f / M_PI;
    mouse_event(MOUSEEVENTF_MOVE, localAngle[0]-orgAngleX, 0, 0, 0);
}
*/

static void getEntities(void)
{
    UINT32 dw_entCnt = 0;
    LPCVOID adr_fstPtr, adr_ent;
    struct g_entity ge;

    adr_fstPtr = readPtrH(adr_firstEntity, ENTFIRST);

#ifdef ENABLE_HRADAR
    radarInvalidateAll();
#endif // ENABLE_HRADAR
    adr_ent = adr_fstPtr;
    while ( adr_ent != NULL )
    {
        memset(&ge, '\0', sizeof(struct g_entity));
        ge.adr = adr_ent;
        if (readEntity(adr_ent, &ge) == TRUE)
        {
            if (dw_entCnt == 0 && (ge.type == ETYPE_PLAYERM || ge.type == ETYPE_PLAYERF))
            {
                localEntitiy = adr_ent;
                localPlayer = ge;
#ifdef ENABLE_HRADAR
                if (localViewAngle[0] != 0.0f && localPlayerStatus != PLAYER_INCAR)
                {
                    radarSetPlayerPosition(localPlayer.pos, localViewAngle[0]);
                }
#endif // ENABLE_HRADAR
            }
#ifndef ENABLE_HRADAR
            printEntity(adr_ent, &ge);
#endif
#ifdef ENABLE_HRADAR
            /*
                        switch (ge.type)
                        {
                        case ETYPE_PLAYERM:
                        case ETYPE_PLAYERF:
                        case ETYPE_ZOMBIE:
                        case ETYPE_ZOMBIE2:
                            if ( (ge.adr != localEntitiy) && ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) )
                            {
                                if (aimed == NULL)
                                {
                                    aimed = &ge;
                                }
                                else
                                {
                                    printf("*** AIMING AT: %s", ge.name);
                                    doAim(ge.pos, localPlayer.pos, localViewAngle);
                                }
                            }
                            else aimed = NULL;
                            break;
                        default:
                            break;
                        }
            */
            drawEntityOnRadar(adr_ent, &ge);
#endif
        }
        else
        {
            printf("ERROR: Could not read (%lu bytes) entity from %p\n", (long unsigned int) sizeof(ge), adr_ent);
        }

        adr_ent = readPtrH(adr_ent, ENTNEXT);
        dw_entCnt++;
    }
#ifdef ENABLE_HRADAR
    radarRemoveInvalidEntities();
#endif // ENABLE_HRADAR
}

static void mainLoop(void)
{
#ifdef ENABLE_HRADAR
    struct radarConfig rad_cfg;
    memset(&rad_cfg, '\0', sizeof(rad_cfg));
    strncpy(rad_cfg.wnd_name, "H1Z1", RDR_NAMELEN);
    radarInit(&rad_cfg);
    radarExecThread();
    radarWaitUntilRdy();
    radarSetDrawDistance(HIZI_MAXDIST);
    rdr_entCnt = radarAddInfo("entities");
#endif // ENABLE_HRADAR
    getEntityCount();
    while (
#ifdef ENABLE_HRADAR
        radarIsActive() == TRUE
#else
        1
#endif
    )
    {
        getEntityCount();
        if (entityCount > 0)
        {
            getEntities();
            localViewAngle[0] = getViewAngleX();
            localViewAngle[1] = getViewAngleY();
            Sleep(100);
#ifdef ENABLE_HRADAR
            radarSetInfoF(rdr_entCnt, "%d", entityCount);
            radarSetPlayerPosition(localPlayer.pos, localViewAngle[0]);
#else
            clrscr();
            printf("H1Z1_HCK...ENTITIES: %lu\n", (long unsigned int) entityCount);
#endif // ENABLE_HRADAR
        }
    }
#ifdef ENABLE_HRADAR
    radarKillThread();
    radarCleanup();
#endif // ENABLE_HRADAR
}

static BOOL getProcessModules(HMODULE hProcess, MODULEINFO *mInfoArr, UINT mInfoSiz, LPCSTR mNames, ...)
{
    BOOL ret = TRUE;
    HMODULE hMods[1024];
    MODULEINFO mod;
    LPCSTR nm = mNames;
    UINT16 found = 0;
    DWORD cbNeeded;
    va_list vrgl;
    int i;

    if (nm == NULL) return FALSE;
    va_start(vrgl, mNames);
    while( nm != NULL && found < mInfoSiz)
    {
        if( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded) )
        {
            for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
            {
                TCHAR szModName[MAX_PATH];
                if ( GetModuleBaseName( hProcess, hMods[i], szModName,
                                        sizeof(szModName) / sizeof(TCHAR)))
                {
                    if ( lstrcmpA(szModName, nm) == 0 )
                    {
                        if ( GetModuleInformation(hProcess, hMods[i], &mod, sizeof(mod)) == TRUE )
                        {
                            mInfoArr[found] = mod;
                            dbg( "*** %s: BaseAddress: 0x%p | EntryPoint: 0x%p | Size: %ld bytes\n", szModName, mInfoArr[found].lpBaseOfDll, mInfoArr[found].EntryPoint, mInfoArr[found].SizeOfImage );
                            break;
                        }
                        else
                        {
                            ret &= FALSE;
                        }
                    }
                }
            }
        }
        nm = va_arg(vrgl, LPCSTR);
        found++;
    }
    va_end(vrgl);
    return TRUE;
}

int _cdecl funcOfDeath(DWORD processID)
{
    BOOL ret = TRUE;
    TCHAR sProc[PROCLEN+1];

    memset(sProc, '\0', PROCLEN+1);

    hndl_proc = OpenProcess( PROCESS_QUERY_INFORMATION |
                             PROCESS_VM_READ | PROCESS_VM_WRITE |
                             PROCESS_VM_OPERATION,
                             TRUE, processID );

    if (hndl_proc != NULL)
    {
        if ( GetModuleBaseName (hndl_proc, NULL, sProc, PROCLEN) > 0 )
        {
            if (lstrcmpA(sProc, "H1Z1.exe") == 0)
            {
                MODULEINFO info;
                dbg("H1Z1.exe found (pid:%ld)", processID);
                if (getProcessModules(hndl_proc, &info, 1, "H1Z1.exe") == TRUE)
                {
                    BYTE *dllBuf = (BYTE *) calloc(info.SizeOfImage, sizeof(BYTE));
                    if ( readMemH( (LPCVOID) info.lpBaseOfDll, dllBuf, sizeof(BYTE)*info.SizeOfImage ) == FALSE )
                    {
                        dbg("%s", "Could not read main module memory.");
                        ret &= FALSE;
                    }
                    else
                    {
                        BYTE rpat[] = GAME_PATTERN;
                        adr_game = getBaseAdr( dllBuf, info.SizeOfImage,
                                               rpat,
                                               GAME_MASK,
                                               BPATTERN_SIZ(rpat),
                                               GAME_OFFSET);
                        adr_game += (ULONG_PTR) info.lpBaseOfDll;
                        //adr_game = 0x142979F20;
                        free(dllBuf);
                        dbg("base pointer: 0x%p", adr_game);
                        mainLoop();
                    }
                }
            }
            else
            {
                ret &= FALSE;
            }
        }
        else
        {
            ret &= FALSE;
        }
    }
    else
    {
        ret &= FALSE;
    }


    CloseHandle( hndl_proc );
    return ret;
}

int _cdecl main(int argc, char **argv)
{
    BOOL ok = FALSE;
    DWORD aProcesses[1024];
    DWORD cbNeeded;
    DWORD cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return 1;
    cProcesses = cbNeeded / sizeof(DWORD);
    for ( i = 0; i < cProcesses; i++ )
    {
        if ( funcOfDeath( aProcesses[i] ) == TRUE )
        {
            printf("done.\n");
            ok = TRUE;
            break;
        }
    }

    if (ok == FALSE)
    {
        printf( "h1z1.exe started and a server joined?\n" );
    }
    return 0;
}
