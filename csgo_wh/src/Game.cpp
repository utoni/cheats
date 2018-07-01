#include <cstdio>
#include <stdlib.h>
#include <math.h>

#include "Game.h"


#ifdef ENABLE_DEBUG
FILE* Game::pLogFile = NULL;
#endif


bool Game::Init(void)
{
    DEBUG_INIT;
    this->init = true;
    if ( (h_clientDLL = GetModuleHandle("client.dll")) == NULL ) return false;
    if ( (h_shaderapiDLL = GetModuleHandle("shaderapidx9.dll")) == NULL ) return false;
    memset(&this->m_client, '\0', sizeof(MODULEINFO));
    memset(&this->m_shaderapi, '\0', sizeof(MODULEINFO));
    if (GetModuleInformation(GetCurrentProcess(), h_clientDLL, &this->m_client, sizeof(MODULEINFO)) == TRUE)
    {
        DEBUG_LOG("MODINFO(client.dll): 0x%p (%lu)", this->m_client.lpBaseOfDll, this->m_client.SizeOfImage);
    }
    else return false;
    if (GetModuleInformation(GetCurrentProcess(), h_shaderapiDLL, &this->m_shaderapi, sizeof(MODULEINFO)) == TRUE)
    {
        DEBUG_LOG("MODINFO(shaderapidx9.dll): 0x%p (%lu)", this->m_shaderapi.lpBaseOfDll, this->m_shaderapi.SizeOfImage);
    }
    else return false;
    return ( true );
}

bool Game::Reset(void)
{
    this->init = false;
    DEBUG_CLOSE;
    return (this->Init());
}

bool Game::ReadEntities(void)
{
    UINT32 i;
    PVOID pLocalPlayer;
    BYTE bLocalPlayerTeam;
    bool bLocalPlayerFound = false;

    if (this->init != true) return false;
    memset(&g_entities, '\0', sizeof(ENTITY)*MAXPLAYER);
    memset(&g_localPlayer, '\0', sizeof(ENTITY));
    pLocalPlayer = (PVOID) *(UINT32*)( (UINT32) this->m_client.lpBaseOfDll + (UINT32) OFF_LOCALPLAYER );
    if ( pLocalPlayer == NULL ) return false;
    bLocalPlayerTeam = *(BYTE*)( (UINT32) pLocalPlayer + ENTTEAM);
    if ( bLocalPlayerTeam != TEAM_COUNTER && bLocalPlayerTeam != TEAM_TERROR ) return false;
    dwPlayerCount = 0;
    for (i = 0; i < MAXPLAYER; i++)
    {

        g_entities[i].p_adr = (PVOID) *(UINT32*)( (UINT32) this->m_client.lpBaseOfDll + (UINT32) OFF_ENTITIES + (UINT32) (ENTLOOP * i) );
        if (g_entities[i].p_adr == NULL)
        {
            break;
        }
        dwPlayerCount++;
    }
    for (i = 0; i < dwPlayerCount; i++)
    {
        if (g_entities[i].p_adr != NULL)
        {
            g_entities[i].p_pos[0] = *(FLOAT *) ( (UINT32) g_entities[i].p_adr + (UINT32) ENTPOS );
            g_entities[i].p_pos[1] = *(FLOAT *) ( (UINT32) g_entities[i].p_adr + (UINT32) ENTPOS + 0x4 );
            g_entities[i].p_pos[2] = *(FLOAT *) ( (UINT32) g_entities[i].p_adr + (UINT32) ENTPOS + 0x8 );

            g_entities[i].p_aim[0] = *(FLOAT *) ( (UINT32) g_entities[i].p_adr + (UINT32) ENTAIMX );
            g_entities[i].p_aim[1] = *(FLOAT *) ( (UINT32) g_entities[i].p_adr + (UINT32) ENTAIMY );
            g_entities[i].p_team = *(BYTE *) ( (UINT32) g_entities[i].p_adr + (UINT32) ENTTEAM );

            if (!bLocalPlayerFound && pLocalPlayer == g_entities[i].p_adr)
            {
                g_localPlayer = &g_entities[i];
                bLocalPlayerFound = true;
            }
        }
        else break;
    }

    return bLocalPlayerFound;
}

void Game::ReadCVars(void)
{
    //dwWidth = *(UINT32*)( (UINT32) this->m_client.lpBaseOfDll + (UINT32) OFF_RESOLUTION );
    //dwHeight = *(UINT32*)( (UINT32) this->m_client.lpBaseOfDll + (UINT32) OFF_RESOLUTION + 0x4 );
    //dwFov = *(UINT32*)( (UINT32) this->m_client.lpBaseOfDll + (UINT32) OFF_FOV );
}

FLOAT Game::calcVecDist(float v1[3], float v2[3])
{
    return sqrtf( powf(v1[0] - v2[0], 2.0f) + powf(v1[1] - v2[1], 2.0f) + powf(v1[2] - v2[2], 2.0f) );
}
