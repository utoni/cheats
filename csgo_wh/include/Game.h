#ifndef GAME_H_INCLUDED
#define GAME_H_INCLUDED

#include <cstdio>
#include <windows.h>
#include <psapi.h>

#define OFF_ENTITIES 0x49EE2E4
#define OFF_LOCALPLAYER 0xA4CA5C

#define MAXPLAYER 32
#define ENTLOOP   0x10
#define ENTPOS    0x134
#define ENTAIMX   0xC0
#define ENTAIMY   0x438
#define ENTVANGL  0x158C
#define ENTTEAM   0xF0
#define PLYFOV    0x159C

#define TEAM_COUNTER 0x3
#define TEAM_TERROR  0x2


#ifdef ENABLE_DEBUG
#define DEBUG_VAR Game::pLogFile
#define DEBUG_FILE "./log.txt"
#define DEBUG_INIT DEBUG_VAR = fopen(DEBUG_FILE,"a+");
#define DEBUG_FLUSH fflush(DEBUG_VAR);
#define DEBUG_CLOSE fclose(DEBUG_VAR);
#define DEBUG_LOG(msg, ...) fprintf(DEBUG_VAR, "[%s:%d] ", __FILE__, __LINE__); fprintf(DEBUG_VAR, msg, __VA_ARGS__); fprintf(DEBUG_VAR, "%s", "\r\n"); DEBUG_FLUSH;
#define WDEBUG_LOG(msg, ...) fwprintf(DEBUG_VAR, L"[%hs:%d] ", __FILE__, __LINE__); fwprintf(DEBUG_VAR, msg, __VA_ARGS__);
#else
#define DEBUG_VAR
#define DEBUG_FILE
#define DEBUG_INIT
#define DEBUG_LOG(msg, ...)
#define WDEBUG_LOG(msg, ...)
#define DEBUG_FLUSH
#define DEBUG_CLOSE
#endif // ENABLE_DEBUG


typedef struct _entitiy
{
    PVOID p_adr;
    FLOAT p_pos[3];
    FLOAT p_aim[2];
    BYTE p_team;
} ENTITY;

class Game
{

private:
    bool init = false;

public:
    HMODULE h_clientDLL, h_shaderapiDLL;
    MODULEINFO m_client, m_shaderapi;
    UINT32 dwWidth;
    UINT32 dwHeight;
    UINT32 dwFov;
    UINT32 dwPlayerCount = 0;
    ENTITY *g_localPlayer;
    ENTITY g_entities[MAXPLAYER];

#ifdef ENABLE_DEBUG
    static FILE* pLogFile;
#endif

    bool Init(void);

    bool Reset(void);

    bool ReadEntities(void);

    void ReadCVars(void);

    static FLOAT calcVecDist(float v1[3], float v2[3]);

};

#endif // GAME_H_INCLUDED
