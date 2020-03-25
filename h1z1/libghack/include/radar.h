#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>

#define HRADAR_FPS 30
#define HRADAR_FONTSIZ 16

enum radarColor {
    RC_RED,
    RC_BLUE,
    RC_GREEN,
    RC_YELLOW,
    RC_CYAN,
    RC_MAGENTA,
    RC_WHITE,
    RC_DONTDRAW
};

struct radarEntity
{
    UINT64 id;
    BOOL valid;
    BOOL isPlayer;

    struct radarEntity *prev;
    struct radarEntity *next;
    void *__internal;
};

#define INFO_LEN 12

struct radarInfo
{
    char *prefix;
    struct radarInfo *next;
    void *__internal;
};

#define RDR_NAMELEN 32

struct radarConfig
{
    char wnd_name[RDR_NAMELEN+1];
};

extern BOOL radarInit(struct radarConfig *rc);

extern void radarCleanup(void);

extern BOOL radarIsActive(void);

extern void radarExecThread(void);

extern void radarKillThread(void);

extern void radarWaitUntilRdy(void);

extern void radarUpdateResolution(void);

extern unsigned int radarGetWidth(void);

extern unsigned int radarGetHeight(void);

extern FLOAT radarPropX(void);

extern FLOAT radarPropY(void);

extern struct radarInfo *radarAddInfo(char *prefix);

extern void radarSetInfo(struct radarInfo *ri, char *text);

extern void radarSetInfoF(struct radarInfo *ri, const char *fmt, ...);

extern void radarSetDrawDistance(FLOAT vecDist);

extern void radarSetPlayerPosition(FLOAT ppos[3], FLOAT angle);

extern void radarSetAimLine(FLOAT enemy_pos3f[3], BOOL enable);

extern void radarUpdateEntity(UINT64 unique_id, FLOAT pos[3], enum radarColor color, BOOL isPlayer, BOOL valid);

extern void radarInvalidateAll(void);

extern void radarRemoveInvalidEntities(void);

extern void radarDrawEntities(void);

extern int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdParam, int cmdShow);

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
