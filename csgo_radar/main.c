/*****************************************
 * Simple CS:GO ExternalRadar/(InGameRadar)Hack/MemDumper
 *
 * 1. playing fair is for loosers
 * 2. fuck your moral!
 * 3. goto 1
 *
 * coded by lnslbrty/dev0, \x90
 *
 * recommendations (mingw32):
 *    CFLAGS=-Wall -DPSAPI_VERSION=1 -DUNICODE=1 -D_UNICODE=1
 *    LDFLAGS=-static-libgcc -static [PATH_TO_LIBPSAPI].a [PATH_TO_LIBKERNEL32].a
 *    Windows PowerShell as Terminal Emulator
 *****************************************/


#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <string.h>
#include <math.h>


#define BPATTERN_SIZ(pat) (sizeof(pat)/sizeof(pat[0]))

/* memory scanning pattern for BASEPOINTER */
#define ENT_BASEPTR_PATTERN { 0xC1, 0xE6, 0x04, 0x81, 0xC6, 0x00, 0x00, 0x00, 0x00, 0x89, 0x45, 0xFC, 0x89, 0x06 }
#define ENT_BASEPTR_MASK    "xxxxx????xxxxx"
#define ENT_BASEPTR_OFFPOS  0x5
#define ENT_BASEPTR_OFFSIZ  0x10
/* " LOCALPLAYER which should be usually YOU */
#define LOCLPLY_PATTERN { 0x03, 0xC0, 0xD5 }
#define LOCLPLY_MASK    "xxx"
#define LOCLPLY_OFFPOS  0x10
/* " RADAR */
#define RADROFF_PATTERN { 0x44, 0xC0, 0xFF, 0xFF, 0xB8, 0x1D, 0xFF, 0xFF, 0xE8, 0xED, 0xFF, 0xFF, 0xE0, 0xCE }
#define RADROFF_MASK    "xx??xx??xx??xx"
#define RADROFF_OFFPOS  0x8C

/* offsets */
/* adr: [client.dll + ENTBASE + (ENTLOOP * entidx)] */
#define ENTLOOP 0x10     // ENTITY POINTER are stored in a field with size 0x10 per entry
/* adr: [entity_adr + TEAMOFF] */
#define TEAMOFF 0xF0     // Team? Terror? Counter? Spectator?
#define HLTHOFF 0xFC     // current health of the entity
#define STTSOFF 0x100    // what does the entity do? crouching? sneaky peaky?
#define ENTPOSA 0x134    // entity position which is a FLOAT[3]
#define ENTVELA 0x110    // entity velocity
#define ENTSPOT 0x935    // is entity spotted? (WRITEABLE!)
#define AWPNOFF 0x12C0   // current weapon offset
#define WEPONID 0x1684   // weapon id: [ [AWPNOFF] + WEPONID ]
#define RADRPTR 0x50     // RADAR POINTER
#define RADNAME 0x24     // name of the radar entity
#define RADSIZE 0x1E0    // radar size is used for looping through the list of radar entities (see ENTLOOP)
#define VECPUNC 0x13DC   // PUNCH VECOTR when some1 or smth hits you (WRITEABLE!)
#define ACCPENL 0x1668   // ACCURACY PENALITY (not WRITEABLE anymore -> is done server side)
#define AIMANGL 0x239C   // entity AIMING ANGLE
#define ACCOUNT 0x238C   // MONEY MONEY MONEY ...
#define ARMORVL 0x2398   // current ARMOE value
#define FLASHAL 0x1da0   // ALPHA value if you get hit by a FLASHBANG (TODO: USE IT FOR HACKING FLASHBANGS!)

#define AIMANGL_MINY -89.0f
#define AIMANGL_MAXY 89.0f
#define AIMANGL_MINX -180.0f
#define AIMANGL_MAXX 180.0f

#define PROCLEN 256
#define MAXPLAYER 32
#define MAXNAMELEN 32
#define RAD_ENEMY_DIST 1000.0f


#define ENTOFF(idx) (DWORD)(baseEntAdr+(ENTLOOP*idx))
#define GET_ENTITY(dllBaseAdr, eidx) readPtr(dllBaseAdr, ENTOFF(eidx))
#define readMem(baseAdr, data, siz) _readMem(baseAdr, data, siz, __FILE__, __LINE__)
#define readPtr(baseAdr, offset) _readPtr(baseAdr, (DWORD) offset, __FILE__, __LINE__)

#ifdef ENABLE_DEBUG
#define DEBUG_VAR pfile
#define DEBUG_FILE "./log.txt"
static FILE *DEBUG_VAR = NULL;
#define DEBUG_INIT DEBUG_VAR = fopen(DEBUG_FILE,"a+"); if (DEBUG_VAR == NULL) perror("fopen");
#define DEBUG_LOG(msg, ...) fprintf(DEBUG_VAR, "[%s:%d] ", __FILE__, __LINE__); fprintf(DEBUG_VAR, msg, __VA_ARGS__);
#define WDEBUG_LOG(msg, ...) fwprintf(DEBUG_VAR, L"[%hs:%d] ", __FILE__, __LINE__); fwprintf(DEBUG_VAR, msg, __VA_ARGS__);
#define DEBUG_FLUSH fflush(DEBUG_VAR);
#define DEBUG_CLOSE fclose(DEBUG_VAR);
#else
#define DEBUG_VAR pfile
#define DEBUG_FILE
#define DEBUG_INIT
#define DEBUG_LOG(msg, ...)
#define WDEBUG_LOG(msg, ...)
#define DEBUG_FLUSH
#define DEBUG_CLOSE
#endif // ENABLE_DEBUG


/* structure for storing pointers to values */
struct off_ent
{
    BOOL valid;
    LPCVOID entBase;
    LPCVOID statusAdr;
    LPCVOID teamAdr;
    LPCVOID healthAdr;
    LPCVOID posAdr;
    LPCVOID radAdr;
    LPCVOID wpnAdr;
    LPCVOID sptAdr;
    LPCVOID velAdr;
    LPCVOID accAdr;
    LPCVOID armAdr;
};
struct off_ent oe[MAXPLAYER];

/* weapon specific stuff like ID and NAME */
#define WPNLEN 5
struct weapon_class
{
    UINT8 wid;
    const wchar_t name[WPNLEN+1];
};
#define WP(id,nm) { id, nm }
struct weapon_class wpns[] =
{
    WP(0 ,L"NONE"), WP(1 ,L"DGLE"), WP(4 ,L"GL18"), WP(2  ,L"DUAL"), WP(36,L"P250"),
    WP(30,L"TEC9"), WP(35,L"NOVA"), WP(25,L"XM10"), WP(29 ,L"SAWD"), WP(14,L"M249"),
    WP(28,L"NEGV"), WP(17,L"MAC1"), WP(33,L" MP7"), WP(24 ,L" UMP"), WP(19,L" P90"),
    WP(26,L"BZON"), WP(42,L"KNFE"), WP(49,L"BOMB"), WP(13 ,L"GLIL"), WP(39,L"SG53"),
    WP(7, L"AK47"), WP(9 ,L"*AWP"), WP(11,L"AUTO"), WP(31 ,L"ZEUS"), WP(44,L"GRND"),
    WP(43,L"FLSH"), WP(47,L"DCOY"), WP(46,L"MOLO"), WP(255,L"NONE"), WP(32,L" P2k"),
    WP(8 ,L"BULL"), WP(16,L"M4A1"), WP(45,L"SMKE"), WP(3  ,L"5SVN"), WP(27,L"MAG7"),
    WP(34,L" MP9"), WP(10,L"FMAS"), WP(38,L"SCAR"), WP(40 ,L" SSG"), WP(48,L"INCD")
};

/* the team */
#define TEAMLEN 5
enum team
{
    TEAM_NONE = 0,
    TEAM_SPECTATOR = 1,
    TEAM_TERROR = 2,
    TEAM_COUNTER = 3,
    TEAM_MAX
};
char team_names[TEAM_MAX][TEAMLEN+1] = { "NONE", "SPEC", "TERR", "CNTR" };

#define ST_SPOTTED 1

/* these are the players (in this case) */
struct g_entity
{
    UINT8 p_team;
    UINT32 p_health;
    UINT32 p_status;
    FLOAT p_pos[3];
    FLOAT p_vel[3];
    wchar_t p_name[MAXNAMELEN+1];
    UINT32 p_wpn;
    UINT32 p_spot;
    UINT32 p_acct;
    UINT32 p_armr;
};
struct g_entity ge[MAXPLAYER]; // not sure if MAXPLAYER is the correct num, but it works


static const int loopDelay = 15;
static const int printDelayFact = 10;
static const int offRescanDelayFact = 250;
static HANDLE hProcess;
static LPCVOID baseEntAdr = NULL, baseLclAdr = NULL, baseRdrAdr = NULL;
static LPCVOID localPlayerAdr = NULL;
static LPCVOID localPlayerPncAdr = NULL;
static LPCVOID localPlayerAccAdr = NULL;
static LPCVOID localPlayerAimAdr = NULL;
static LPCVOID localPlayerFlsAdr = NULL;
static FLOAT vecPunch[2] = { 0.0f, 0.0f };
static FLOAT accuracyPenalty = 0.0f;
static FLOAT aimAngle[2] = { 0.0f, 0.0f };
static FLOAT flashAlpha = 0.0f;  // TODO: disable Flashbangs ;)
static struct g_entity *localPlayer = NULL;


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

static inline LPCVOID calcOffset(LPCVOID baseAdr, DWORD offset)
{
    LPCVOID adr = (LPCVOID)( (size_t)baseAdr+(size_t)offset );
    return adr;
}

static inline BOOL _readMem(LPCVOID adr, PVOID data, DWORD siz, char *srcFile, size_t lineNmb)
{
    SIZE_T rpmBytesRead = 0;
    if ( ReadProcessMemory( hProcess, adr, data, siz, &rpmBytesRead ) == FALSE )
    {
        DWORD errCode = GetLastError();
        printf( "\n[%s:%lu] ReadProcessMemory@(0x%p) FAILED (read: %lu/%lu byte) with ERROR code: %ld!\n", srcFile, (long unsigned int) lineNmb, adr, rpmBytesRead, siz, errCode );
        if (errCode == 299)
        {
            fprintf(stderr, "I don't know what to do ... exiting!\n");
            exit(1);
        }
        return FALSE;
    }
    return TRUE;
}

static inline LPCVOID _readPtr(LPCVOID baseAdr, DWORD offset, char *srcFile, size_t lineNmb)
{
    LPCVOID adr = calcOffset( (DWORD *) baseAdr, offset );
    LPCVOID ptr;
    if (_readMem(adr, &ptr, sizeof(LPCVOID), srcFile, lineNmb) == TRUE)
    {
        return ptr;
    }
    return NULL;
}

static SIZE_T findPattern(BYTE *b, const BYTE *p, const char *mask, SIZE_T b_siz, SIZE_T pm_siz)
{
    SIZE_T i,j;

    j = 0;
    for (i = 0; i < b_siz; i++)
    {
        if ( *(char *)(mask + j) == 'x' )
        {
            if ( *(BYTE *)(b + i) == *(BYTE *)(p + j) )
            {
                j++;
                if ( j == pm_siz ) return (i - pm_siz + 1);
            }
            else
            {
                j = 0;
            }
        }
        else j++;
        if (j == pm_siz)
        {
            j = 0;
        }
    }
    return i;
}

static LPCVOID getBaseAdr(LPCVOID startAdr, BYTE *buf, SIZE_T siz, const BYTE *pat, const char *mask, const SIZE_T pm_siz, const SIZE_T off_pos)
{
    SIZE_T pos = 0;

    if ( (pos = findPattern(&buf[0], &pat[0], &mask[0], siz, pm_siz)) <= (siz - pm_siz) )
    {
        pos += off_pos;
    }
    else
    {
        DEBUG_LOG("Could not find base pointer: %lu/%lu\n", pos, siz);
        DEBUG_LOG("  Mask   : %s\n", mask);
        DEBUG_FLUSH
        return 0x0;
    }
    return ( (LPCVOID) pos );
}

static UINT8 getEntityCount(struct off_ent *oe, LPCVOID dllBaseAdr, char *printable_name, UINT8 *invalid_ents)
{
    LPCVOID entAdr;
    int i;
    (*invalid_ents) = 0;
    for (i = 0; i < MAXPLAYER; i++)
    {
        entAdr = GET_ENTITY( dllBaseAdr, i );
        if (entAdr == NULL)
        {
            (*invalid_ents)++;
            oe[i].valid = FALSE;
            continue;
        }
        else
        {
            oe[i].valid = TRUE;
        }
    }
    if ( (*invalid_ents) == (UINT8) MAXPLAYER ) return 0xFF;
    return i;
}

static BOOL getEntityOffs(LPCVOID dllBaseAdr, int eidx, struct off_ent *e)
{
    BOOL ret = TRUE;
    LPCVOID adr;
    LPCVOID data;
    if ( (data = GET_ENTITY( dllBaseAdr, eidx )) != NULL)
    {
        if (e->entBase != data)
        {
            WDEBUG_LOG( L"ENTITY(%d) BASE ADR CHANGED: [0x%p + 0x%08X] = 0x%p\n", eidx, dllBaseAdr, (unsigned int)ENTOFF(eidx), data);
        }
        e->entBase = data;
        e->teamAdr = calcOffset( data, TEAMOFF );
        e->healthAdr = calcOffset( data, HLTHOFF );
        e->statusAdr = calcOffset( data, STTSOFF );
        e->posAdr = calcOffset( data, ENTPOSA );
        e->wpnAdr = calcOffset( data, AWPNOFF );
        e->sptAdr = calcOffset( data, ENTSPOT );
        e->velAdr = calcOffset( data, ENTVELA );
        e->accAdr = calcOffset( data, ACCOUNT );
        e->armAdr = calcOffset( data, ARMORVL );
    }
    else
    {
        WDEBUG_LOG( L"ENTITY(%d) BASE ADR INVALID: [0x%p + 0x%08X]\n", eidx, dllBaseAdr, (unsigned int)ENTOFF(eidx) );
        ret &= FALSE;
    }
    if ( (data = readPtr( dllBaseAdr, baseRdrAdr )) != NULL)
    {
        adr = calcOffset( data, RADRPTR );
        if (readMem(adr, &data, sizeof(data)) == TRUE)
        {
            adr = calcOffset( data, (RADSIZE * (eidx + 1)) + RADNAME );
            e->radAdr = adr;
        }
    }
    else ret &= FALSE;
    if ( localPlayerAdr == NULL && (data = readPtr( dllBaseAdr, baseLclAdr )) != NULL )
    {
        localPlayerAdr = data;
        localPlayerAccAdr = calcOffset( localPlayerAdr, ACCPENL );
        localPlayerPncAdr = calcOffset( localPlayerAdr, VECPUNC );
        localPlayerAimAdr = calcOffset( localPlayerAdr, AIMANGL );
        localPlayerFlsAdr = calcOffset( localPlayerAdr, FLASHAL );
    }
    return ret;
}

static BOOL getEntityInfo(LPCVOID dllBaseAdr, struct off_ent *oe, struct g_entity *ge)
{
    BOOL ret = TRUE;
    LPCVOID data, adr;
    float tmpAngl[2];

    ret &= readMem(oe->teamAdr, &ge->p_team, sizeof(UINT32));
    ret &= readMem(oe->healthAdr, &ge->p_health, sizeof(UINT32));
    ret &= readMem(oe->statusAdr, &ge->p_status, sizeof(UINT8));
    ret &= readMem(oe->posAdr, &ge->p_pos[0], sizeof(FLOAT)*3);
    ret &= readMem(oe->radAdr, &ge->p_name, sizeof(wchar_t)*MAXNAMELEN);
    ret &= readMem(oe->wpnAdr, &ge->p_wpn, sizeof(UINT32));
    ret &= readMem(oe->sptAdr, &ge->p_spot, sizeof(UINT32));
    ret &= readMem(oe->velAdr, &ge->p_vel[0], sizeof(FLOAT)*3);
    ret &= readMem(oe->accAdr, &ge->p_acct, sizeof(UINT32));
    ret &= readMem(oe->armAdr, &ge->p_armr, sizeof(UINT32));
    ge->p_wpn &= 0xFFF; // mask some bits to get the correct ID, thx gaben
    if ( (data = readPtr(dllBaseAdr, (DWORD)baseEntAdr+ENTLOOP*(ge->p_wpn-1))) != NULL )
    {
        adr = calcOffset( data, WEPONID );
        ret &= readMem(adr, &ge->p_wpn, sizeof(UINT32));
    }
    if (oe->entBase == localPlayerAdr)
    {
        localPlayer = ge;
        ret &= readMem(localPlayerAccAdr, &accuracyPenalty, sizeof(FLOAT));
        ret &= readMem(localPlayerPncAdr, &vecPunch, sizeof(FLOAT)*2);
        ret &= readMem(localPlayerAimAdr, &tmpAngl, sizeof(FLOAT)*2);
        ret &= readMem(localPlayerFlsAdr, &flashAlpha, sizeof(FLOAT));
        if (tmpAngl[0] <= AIMANGL_MAXY && tmpAngl[0] >= AIMANGL_MINY)
        {
            aimAngle[0] = tmpAngl[0];
        }
        if (tmpAngl[1] <= AIMANGL_MAXX && tmpAngl[1] >= AIMANGL_MINX)
        {
            aimAngle[1] = tmpAngl[1];
        }
    }
    return ret;
}

static void printPrettyEntityInfoHdr(void)
{
    clrscr();
    printf( "\nTEAM | WEPN | STATUS | HEALTH | POSITION[X,Y,Z]              | VELOCITY[X,Y,Z]              | VECDIST  | MONEY | ARMOR | NAME\n" );
}

static void getWeaponName(UINT8 wid, wchar_t wp[WPNLEN+1])
{
    int i;
    for (i = 0; i < (sizeof(wpns)/sizeof(wpns[0])); i++)
    {
        if (wpns[i].wid == wid)
        {
            snwprintf(wp, WPNLEN, L"%s", wpns[i].name);

            return;
        }
    }
    snwprintf(wp, WPNLEN, L"%04u", wid);
}

static float calcVecDist(float v1[3], float v2[3])
{
    return sqrtf( powf(v1[0] - v2[0], 2.0f) + powf(v1[1] - v2[1], 2.0f) + powf(v1[2] - v2[2], 2.0f) );
}

static void printPrettyEntityInfo(struct g_entity *ge)
{
    wchar_t tm[TEAMLEN+1];
    wchar_t wp[WPNLEN+1];

    memset(tm, '\0', (TEAMLEN+1) * sizeof(tm[0]));
    memset(wp, '\0', WPNLEN+1 * sizeof(wp[0]));
    switch (ge->p_team)
    {
    case TEAM_NONE:
    case TEAM_COUNTER:
    case TEAM_TERROR:
    case TEAM_SPECTATOR:
        snwprintf(tm, TEAMLEN, L"%hs", team_names[ge->p_team]);
        break;
    case TEAM_MAX:
    default:
        snwprintf(tm, TEAMLEN, L"%04u", ge->p_team);
        break;
    }

    getWeaponName(ge->p_wpn, wp);

    if (ge->p_health == 0)
    {
        wprintf( L"%s | %s | [ %02d ] | [ %03d] | %+8.2f, %+8.2f, %+8.2f | %+8.2f, %+8.2f, %+8.2f | %8.2f | %05lu |   %03lu | %ls\n",
                 tm, wp,
                 ge->p_status, ge->p_health,
                 0.0f, 0.0f, 0.0f,
                 0.0f, 0.0f, 0.0f,
                 0.0f,
                 ge->p_acct, 0,
                 ge->p_name);
    }
    else
    {
        wprintf( L"%s | %s | [ %02d ] | [ %03d] | %+8.2f, %+8.2f, %+8.2f | %+8.2f, %+8.2f, %+8.2f | %8.2f | %05lu |   %03lu | %ls %s",
                 tm, wp,
                 ge->p_status, ge->p_health,
                 ge->p_pos[0], ge->p_pos[1], ge->p_pos[2],
                 ge->p_vel[0], ge->p_vel[1], ge->p_vel[2],
                 (localPlayer != NULL ? calcVecDist(localPlayer->p_pos, ge->p_pos) : 0.0f),
                 ge->p_acct, ge->p_armr,
                 ge->p_name,
                 (ge->p_spot == ST_SPOTTED ? L" * SPOTTED\n" : L"\n")
               );
    }
}

static void printAllPrettyEntityInfos(struct off_ent oe[MAXPLAYER], struct g_entity ge[MAXPLAYER], enum team tm)
{
    int i;
    for (i = 0; i < MAXPLAYER; i++)
    {
        if (oe[i].valid == TRUE)
        {
            if (ge[i].p_team == tm)
            {
                printPrettyEntityInfo(&ge[i]);
            }
        }
    }
}

static BOOL GetMessageWithTimeout(MSG *msg, UINT to)
{
    BOOL res;
    UINT_PTR timerId = SetTimer(NULL, 0, to, NULL);
    res = GetMessage(msg, NULL, 0, 0);
    KillTimer(NULL, timerId);
    if (!res)
        return FALSE;
    if (msg->message == WM_TIMER && msg->hwnd == NULL && msg->wParam == timerId)
        return FALSE;
    return TRUE;
}

static BOOL showEntityOnRad(struct off_ent *oe, struct g_entity *ge, BOOL activate)
{
    UINT32 spot = (activate == TRUE ? 0x1 : 0x0);
    BOOL ret = TRUE;
    if (localPlayer->p_team != TEAM_COUNTER && localPlayer->p_team != TEAM_TERROR) return FALSE;
    if (oe->valid == FALSE) return TRUE;
    if (ge->p_team == localPlayer->p_team) return TRUE;
    if ( (ge->p_spot != ST_SPOTTED && activate == TRUE) || (ge->p_spot == ST_SPOTTED && activate == FALSE) )
    {
        if ( WriteProcessMemory( hProcess, (LPVOID) oe->sptAdr, &spot, sizeof(spot), NULL ) == FALSE )
        {
            printf("*** ERROR WRITING INTO MEMORY: %ld ***\n", GetLastError());
            ret &= FALSE;
        }
    }
    return ret;
}

static BOOL showAllEnemiesOnRad(struct off_ent oe[MAXPLAYER], struct g_entity ge[MAXPLAYER], BOOL activate)
{
    int i;
    for (i = 0; i < MAXPLAYER; i++)
    {
        if (showEntityOnRad(&oe[i], &ge[i], activate) == FALSE)
        {
            return FALSE;
        }
    }
    return TRUE;
}

static BOOL reduceRecoil(void)
{
    FLOAT normAngles[2];
    FLOAT puncAngles[2] = { vecPunch[0] * 1.97f, vecPunch[1] * 1.97f };

    if (abs(puncAngles[0]) < 1.0f && abs(puncAngles[1]) < 0.5f) return TRUE;
    normAngles[0] = aimAngle[0] - puncAngles[0];
    normAngles[1] = aimAngle[1] - puncAngles[1];
    if (normAngles[0] != aimAngle[0] ||normAngles[1] != aimAngle[1])
    {
        //printf("****** %+8.2f , %+8.2f ", vecPunch[0] * 1.97f, vecPunch[1] * 1.97f);
        mouse_event(0x1, vecPunch[1]*-1.0f, vecPunch[0]*-0.19f, 0, 0);
    }
    return TRUE;
}

static UINT8 scanEntities(LPCVOID baseAdr, struct off_ent oe[MAXPLAYER])
{
    int i;
    UINT8 entcnt = 0;
    UINT8 invalid_ents = 0;

    while ( (entcnt = getEntityCount( oe, (DWORD *)baseAdr, NULL, &invalid_ents )) == 0xFF )
    {
        printf(".");
        Sleep(500);
    }

    for (i = 0; i < MAXPLAYER; i++)
    {
        if (oe[i].valid == TRUE)
        {
            if ( getEntityOffs( (DWORD *)baseAdr, i, &oe[i] ) == FALSE )
            {
                WDEBUG_LOG( L"ENTITY(%d) IS NOW INVALID: [0x%p + 0x%08X]\n", i, baseAdr, (unsigned int)ENTOFF(i) );
                oe[i].valid = FALSE;
            }
        }
    }
    return entcnt;
}

static BOOL getProcessModules(HMODULE hProcess, MODULEINFO *mInfoArr, UINT mInfoSiz, LPCWSTR mNames, ...)
{
    BOOL ret = TRUE;
    HMODULE hMods[1024];
    MODULEINFO mod;
    LPCWSTR nm = mNames;
    UINT16 found = 0;
    DWORD cbNeeded;
    va_list vrgl;
    int i;

    if (nm == NULL) return FALSE;
    va_start(vrgl, mNames);
    while ( nm != NULL && found < mInfoSiz)
    {
        if ( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded) )
        {
            for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
            {
                TCHAR szModName[MAX_PATH];
                if ( GetModuleBaseName( hProcess, hMods[i], szModName,
                                        sizeof(szModName) / sizeof(TCHAR)))
                {
                    if ( lstrcmpW(szModName, nm) == 0 )
                    {
                        if ( GetModuleInformation(hProcess, hMods[i], &mod, sizeof(mod)) == TRUE )
                        {
                            mInfoArr[found] = mod;
                            WDEBUG_LOG( L"*** %s: BaseAddress: 0x%p | EntryPoint: 0x%p | Size: %ld bytes\n", szModName, mInfoArr[found].lpBaseOfDll, mInfoArr[found].EntryPoint, mInfoArr[found].SizeOfImage );
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
        nm = va_arg(vrgl, LPCWSTR);
        found++;
    }
    va_end(vrgl);
    return TRUE;
}

int doHack( DWORD processID )
{
    BOOL mapHackSuccess = FALSE;
    MODULEINFO clientMod;
    BYTE *dllBuf = NULL;
    SIZE_T codeIdx = 0;
    TCHAR sProc[PROCLEN+1];
    unsigned int i;
    DWORD cnt = 0;

    memset(sProc, '\0', (PROCLEN+1) * sizeof(sProc[0]));
    memset(oe, '\0', MAXPLAYER*sizeof(struct off_ent));
    memset(ge, '\0', MAXPLAYER*sizeof(struct g_entity));

    hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                            PROCESS_VM_READ | PROCESS_VM_WRITE |
                            PROCESS_VM_OPERATION,
                            TRUE, processID );
    if (NULL == hProcess)
    {
        goto error;
    }

    if ( GetModuleBaseName ( hProcess, NULL, sProc, PROCLEN) > 0 )
    {
        if (lstrcmp(sProc, L"csgo.exe") == 0)
        {
            DEBUG_LOG("*** csgo.exe found: %ld\n", processID);
        }
        else
        {
            goto error;
        }
    }
    else
    {
        goto error;
    }

    if ( getProcessModules(hProcess, &clientMod, 1, L"client.dll") == TRUE )
    {
        dllBuf = calloc(clientMod.SizeOfImage, sizeof(BYTE));
        printf("Searching Basepointer: ");
        while (1)
        {
            if ( dllBuf != NULL && readMem(clientMod.lpBaseOfDll, dllBuf, clientMod.SizeOfImage) == TRUE )
            {
                BYTE epat[] = ENT_BASEPTR_PATTERN;
                codeIdx = (SIZE_T) getBaseAdr(clientMod.lpBaseOfDll, dllBuf, clientMod.SizeOfImage,
                                              epat,
                                              ENT_BASEPTR_MASK,
                                              BPATTERN_SIZ(epat),
                                              ENT_BASEPTR_OFFPOS);
                baseEntAdr = (LPCVOID) ( *(long unsigned int *)(dllBuf + codeIdx) + ENT_BASEPTR_OFFSIZ - (long unsigned int)clientMod.lpBaseOfDll );
                //baseEntAdr = 0x04A13264;
                printf("0x%p, ", baseEntAdr);
                BYTE lpat[] = LOCLPLY_PATTERN;
                baseLclAdr = getBaseAdr(clientMod.lpBaseOfDll, dllBuf, clientMod.SizeOfImage,
                                        lpat,
                                        LOCLPLY_MASK,
                                        BPATTERN_SIZ(lpat),
                                        LOCLPLY_OFFPOS);
                baseLclAdr = (LPCVOID) 0xA4CA5C;
                printf("0x%p, ", baseLclAdr);
                BYTE rpat[] = RADROFF_PATTERN;
                baseRdrAdr = getBaseAdr(clientMod.lpBaseOfDll, dllBuf, clientMod.SizeOfImage,
                                        rpat,
                                        RADROFF_MASK,
                                        BPATTERN_SIZ(rpat),
                                        RADROFF_OFFPOS);
                printf("0x%p\n", baseRdrAdr);
                baseRdrAdr = (LPCVOID) 0x49EE2E4;
                break;
            }
            Sleep(5000);
        }
        free(dllBuf);
        dllBuf = NULL;
        printf("Scanning entities ");
        scanEntities(clientMod.lpBaseOfDll, oe);
        while (1)
        {
            for (i = 0; i < MAXPLAYER; i++)
            {
                if (oe[i].valid == FALSE) continue;
                if (getEntityInfo((DWORD *)clientMod.lpBaseOfDll, &oe[i], &ge[i]) != TRUE)
                {
                    goto error;
                }
                if (ge[i].p_team < 0 || ge[i].p_team > 3 || (cnt % offRescanDelayFact) == 0)
                {
                    DEBUG_FLUSH;
                    localPlayerAdr = NULL;
                    scanEntities(clientMod.lpBaseOfDll, oe);
                }
                if (localPlayer != NULL)
                {
                    if (calcVecDist(localPlayer->p_pos, ge[i].p_pos) < RAD_ENEMY_DIST)
                    {
                        showEntityOnRad(&oe[i], &ge[i], TRUE);
                    }
                    else
                    {
                        showEntityOnRad(&oe[i], &ge[i], FALSE);
                    }
                }
            }

            if ( (cnt % printDelayFact) == 0 )
            {
                // print table header
                printPrettyEntityInfoHdr();
                printf("\n----------------------------[ TERROR  ]----------------------------\n\n");
                // terror
                printAllPrettyEntityInfos(oe, ge, TEAM_TERROR);
                printf("\n\n\n----------------------------[ COUNTER ]----------------------------\n\n");
                // counter
                printAllPrettyEntityInfos(oe, ge, TEAM_COUNTER);
                printf("\n\n\n------------------------[ SPECTATOR/NONE ]-------------------------\n\n");
                // spectacter/none
                printAllPrettyEntityInfos(oe, ge, TEAM_NONE | TEAM_SPECTATOR);

                if (localPlayer == NULL)
                {
                    printf("*** RADAR DISABLED (NO LOCAL PLAYER FOUND) ***\n");
                }
                else
                {
                    MSG msg = {0};
                    if (GetMessageWithTimeout(&msg, 1))
                    {
                        if (msg.message == WM_HOTKEY)
                        {
                            mapHackSuccess = showAllEnemiesOnRad(oe, ge, TRUE);
                        }
                    }
                    printf("\n\n[LOCALPLAYER] VECPUNCH[x,y]: [%+8.2f,%+8.2f] | ACCPENLTY: %+8.2f | AIMANGLES[x,y]: [%+8.2f,%+8.2f]\n", vecPunch[1], vecPunch[0], accuracyPenalty, aimAngle[1], aimAngle[0]);
                }

                printf("\n\n[BASE_PTRs] ENTITY: 0x%p | LOCALPLAYER: 0x%p | RADAR: 0x%p\n\n", baseEntAdr, baseLclAdr, baseRdrAdr);
            }
            cnt++;
            Sleep(loopDelay);
            if (mapHackSuccess == TRUE) showAllEnemiesOnRad(oe, ge, FALSE);
            reduceRecoil();
        }
    }
    else
    {
        goto error;
    }

    CloseHandle( hProcess );
    return 0;
error:
    CloseHandle( hProcess );
    return 1;
}

int main( int argc, char **argv )
{
    BOOL ok = FALSE;
    DWORD aProcesses[1024];
    DWORD cbNeeded;
    DWORD cProcesses;
    unsigned int i;

    DEBUG_INIT;
    if (RegisterHotKey(NULL, 1, 0x4000, VK_F1))
    {
        DEBUG_LOG( "*** Hotkey 'F1' registered (show enemies on radar for %dms) ..\n", loopDelay );
    }
    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return 1;
    cProcesses = cbNeeded / sizeof(DWORD);
    for ( i = 0; i < cProcesses; i++ )
    {
        if ( doHack( aProcesses[i] ) == 0 )
        {
            DEBUG_LOG("%s", "*** FOUND!\n");
            ok = TRUE;
            break;
        }
    }

    if (ok == FALSE)
    {
        printf( "csgo.exe started and a game joined?\n" );
    }
    DEBUG_CLOSE;
    return 0;
}
