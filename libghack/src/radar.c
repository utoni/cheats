/*****************************************
 * hRadar (C) 2015 by lnslbrty/dev0, \x90
 *
 * Simple, C coded, 2D-SFML API, ...
 *
 * Happy Hacking!
 *****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <inttypes.h>
#include <math.h>
#include <SFML/Graphics.h>
#include <SFML/Audio.h>

#include "ghack.h"


#define WTITLE_SUFFIX "hRadar"
#define AIMANGLE_LINELEN 35.0f
#define AIMLINE_WIDTH 1.3f
#define ENTITY_CIRCLE_RADIUS 3.5f

static BOOL radarActive = TRUE;
static sfThread *sThrd;
static sfVideoMode vMode;
static sfContextSettings cSettings;
static sfRenderWindow *rWnd = NULL;
static sfFont *defFont = NULL;
static char *wTitle = NULL;
static unsigned int gr_width = 0;
static unsigned int gr_height = 0;
static FLOAT f_vecDistance = 0.0f, f_propX = 1.0f, f_propY = 1.0f;
static FLOAT f_angle = 0.0f;
static FLOAT f_playerPos[3] = { 0.0f, 0.0f, 0.0f };
static struct radarInfo *rdr_ver = NULL;
static sfRectangleShape *rdr_crosshair[2];
static sfRectangleShape *aimLine = NULL;
static sfRectangleShape *rdr_angle = NULL;
static HANDLE mtx_set = NULL;
static HANDLE ev_ready = NULL;

static struct radarEntity *rad_entityList = NULL;
static struct radarInfo *rad_info = NULL;
#define DEF_RADINFO_X 5.0f
#define DEF_RADINFO_STARTY 2.0f
#define DEF_RADINFO_DY 18.0f
static unsigned int d_radDefY = DEF_RADINFO_STARTY;

//#undef ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#define DEBUG_VAR pfile
#define DEBUG_FILE "./log.txt"
static FILE *DEBUG_VAR = NULL;
#define DEBUG_INIT DEBUG_VAR = fopen(DEBUG_FILE,"a+"); if (DEBUG_VAR == NULL) perror("fopen");
#define DEBUG_LOG(msg, ...) fprintf(DEBUG_VAR, "[%s:%d] ", __FILE__, __LINE__); fprintf(DEBUG_VAR, msg, __VA_ARGS__);
#define DEBUG_FLUSH fflush(DEBUG_VAR);
#define DEBUG_CLOSE fclose(DEBUG_VAR);
#else
#define DEBUG_VAR
#define DEBUG_FILE
#define DEBUG_INIT
#define DEBUG_LOG(msg, ...)
#define DEBUG_FLUSH
#define DEBUG_CLOSE
#endif // ENABLE_DEBUG

struct graphicEnt
{
    sfCircleShape *cr_entShape;
};

static inline char *strDupExt(char *src, SIZE_T siz)
{
    char *txt = calloc(siz+1, sizeof(char));

    memcpy(txt, src, siz);
    return txt;
}

static struct radarEntity *radarNewEntity(UINT64 id, BOOL isPlayer, BOOL valid)
{
    struct radarEntity *re = calloc(1, sizeof(struct radarEntity));
    struct graphicEnt *ge = calloc(1, sizeof(struct graphicEnt));

    re->__internal = ge;
    re->valid = valid;
    re->isPlayer = isPlayer;
    re->id = id;
    ge->cr_entShape = sfCircleShape_create();
    sfCircleShape_setRadius(ge->cr_entShape, ENTITY_CIRCLE_RADIUS);
    return re;
}

static void radarDeleteEntity(struct radarEntity *re)
{
    struct graphicEnt *ge = (struct graphicEnt *) re->__internal;

    if (ge != NULL)
    {
        sfCircleShape_destroy(ge->cr_entShape);
        free(ge);
    }
    free(re);
}

static sfColor radarSFMLColor(enum radarColor color)
{
    switch (color)
    {
    case RC_RED:
            return sfRed;
    case RC_BLUE:
        return sfBlue;
    case RC_GREEN:
        return sfGreen;
    case RC_YELLOW:
        return sfYellow;
    case RC_WHITE:
        return sfWhite;
    case RC_MAGENTA:
        return sfMagenta;
    case RC_CYAN:
        return sfCyan;
    case RC_DONTDRAW:
    default:
        break;
    }
    return sfWhite;
}

static BOOL _radarInit(void *arg)
{
    struct radarConfig *rdr = (struct radarConfig *) arg;

    DEBUG_INIT
    DEBUG_LOG("%s", "initialized\n");
    DEBUG_FLUSH
    wTitle = calloc(strlen(WTITLE_SUFFIX) + strlen(rdr->wnd_name) + 4, sizeof(char)); // + 4: " - " + \0
    strcat(wTitle, rdr->wnd_name);
    strcat(wTitle, " - ");
    strcat(wTitle, WTITLE_SUFFIX);

    mtx_set = CreateMutexA(NULL, FALSE, NULL);
    ev_ready = CreateEventA(NULL, FALSE, FALSE, "ev_hrad_ready");
    memset(&cSettings, '\0', sizeof(cSettings));
    cSettings.majorVersion = 2;
    cSettings.depthBits = 32;
    defFont = sfFont_createFromFile("C:\\Windows\\Fonts\\cour.ttf");
    rad_entityList = radarNewEntity(0x0, FALSE, FALSE);
    rad_info = calloc(1, sizeof(struct radarInfo));
    return (rWnd != NULL ? TRUE : FALSE);
}

extern BOOL radarInit(struct radarConfig *rc)
{
    return _radarInit(rc);
}

extern BOOL radarIsActive(void)
{
    return radarActive;
}

static void radarInfoCleanup(void)
{
    struct radarInfo *cur, *next;

    next = rad_info->next;
    while ( (cur = next) != NULL )
    {
        if (cur->__internal != NULL) sfText_destroy(cur->__internal);
        next = cur->next;
        if (cur->prefix) free(cur->prefix);
        free(cur);
    }
    d_radDefY = DEF_RADINFO_STARTY;
    free(rad_info);
}

extern void radarCleanup(void)
{
    if (wTitle == NULL) return;
    CloseHandle(mtx_set);
    CloseHandle(ev_ready);
    sfRenderWindow_destroy(rWnd);
    radarInfoCleanup();
    sfFont_destroy(defFont);
    rWnd = NULL;
    free(wTitle);
    wTitle = NULL;
    DEBUG_CLOSE
}

extern void radarWaitUntilRdy(void)
{
    WaitForSingleObject(ev_ready, INFINITE);
}

extern void radarReleaseMutex(void)
{
    ReleaseMutex(mtx_set);
}

extern void radarSetMutex(void)
{
    WaitForSingleObject(mtx_set, INFINITE);
}

static void radarInfoSetPos(sfText *sf, FLOAT x, FLOAT y)
{
    sfVector2f tf;

    tf.x = x;
    tf.y = y;
    sfText_setPosition(sf, tf);
}

static void radarInfoSet(sfText *st, char *s, unsigned int c_size, enum radarColor color)
{
    sfText_setString(st, s);
    sfText_setColor(st, radarSFMLColor(color));
    sfText_setCharacterSize(st, c_size);
    sfText_setFont(st, defFont);
    sfText_setStyle(st, sfTextBold);
}

static void radarDrawInfo(void)
{
    struct radarInfo *ri = rad_info;

    while ( (ri = ri->next) != NULL )
    {
        sfRenderWindow_drawText(rWnd, ri->__internal, NULL);
    }
}

static sfRectangleShape *radarDrawLine(FLOAT x, FLOAT y, FLOAT width, FLOAT angle, sfColor color)
{
    sfVector2f v1, v2;
    v1.x = x;
    v1.y = y;
    v2.x = width;
    v2.y = 1.0f;

    sfRectangleShape *rct = sfRectangleShape_create();
    sfRectangleShape_setPosition(rct, v1);
    sfRectangleShape_setSize(rct, v2);
    sfRectangleShape_setRotation(rct, angle);
    sfRectangleShape_setFillColor(rct,  color);
    return rct;
}

static void _radarLoop(void *arg)
{
    radarUpdateResolution();
    rWnd = sfRenderWindow_create(vMode, wTitle, sfDefaultStyle, &cSettings);
    sfRenderWindow_setFramerateLimit(rWnd, HRADAR_FPS);
    rdr_ver = radarAddInfo("version");
    radarSetInfo(rdr_ver, GHACK_VERSION);
    rdr_crosshair[0] = radarDrawLine(0.0f, gr_height/2, gr_width, 0.0f, radarSFMLColor(RC_RED));
    rdr_crosshair[1] = radarDrawLine(gr_width/2, 0, gr_height, 90.0f, radarSFMLColor(RC_RED));
    aimLine = radarDrawLine(0.0f, 0.0f, 0.0f, 0.0f, radarSFMLColor(RC_MAGENTA));
    rdr_angle = radarDrawLine(gr_width/2, gr_height/2, AIMANGLE_LINELEN, 45.0f, radarSFMLColor(RC_MAGENTA));
    sfRectangleShape_setFillColor(rdr_angle, radarSFMLColor(RC_WHITE));

    radarActive = TRUE;
    while (sfRenderWindow_isOpen(rWnd) == TRUE)
    {
        sfEvent ev;
        while (sfRenderWindow_pollEvent(rWnd, &ev) == TRUE)
        {
            if (ev.type == sfEvtClosed)
            {
                radarActive = FALSE;
            }
            else if (ev.type == sfEvtResized)
            {
                radarUpdateResolution();
            }
        }
        sfRenderWindow_clear(rWnd, sfBlack);
        sfRenderWindow_drawRectangleShape(rWnd, rdr_crosshair[0], NULL);
        sfRenderWindow_drawRectangleShape(rWnd, rdr_crosshair[1], NULL);
        sfRenderWindow_drawRectangleShape(rWnd, aimLine, NULL);
        WaitForSingleObject(mtx_set, INFINITE);
        sfRectangleShape_setRotation(rdr_angle, f_angle);
        sfRenderWindow_drawRectangleShape(rWnd, rdr_angle, NULL);
        radarDrawEntities();
        radarDrawInfo();
        ReleaseMutex(mtx_set);

        sfRenderWindow_display(rWnd);
        SetEvent(ev_ready);
    }
    sfRectangleShape_destroy(rdr_crosshair[0]);
    sfRectangleShape_destroy(rdr_crosshair[1]);
    sfRectangleShape_destroy(rdr_angle);
}

extern void radarUpdateResolution(void)
{
    WaitForSingleObject(mtx_set, INFINITE);
    vMode = sfVideoMode_getDesktopMode();
    gr_width = vMode.width;
    gr_height = vMode.height;
    ReleaseMutex(mtx_set);
}

extern unsigned int radarGetWidth(void)
{
    return gr_width;
}

extern unsigned int radarGetHeight(void)
{
    return gr_height;
}

extern FLOAT radarPropX(void)
{
    return f_propX;
}

extern FLOAT radarPropY(void)
{
    return f_propY;
}

extern struct radarInfo *radarAddInfo(char *prefix)
{
    struct radarInfo *ri = rad_info;

    WaitForSingleObject(mtx_set, INFINITE);
    while ( ri->next != NULL )
    {
        ri = ri->next;
    }
    ri->next = calloc(1, sizeof(struct radarInfo));
    ri->next->__internal = sfText_create();
    ri->next->prefix = strDupExt(prefix, INFO_LEN);
    radarInfoSetPos(ri->next->__internal, DEF_RADINFO_X, d_radDefY);
    d_radDefY += DEF_RADINFO_DY;
    ReleaseMutex(mtx_set);
    return ri->next;
}

extern void radarSetInfo(struct radarInfo *ri, char *text)
{
    char tbuf[INFO_LEN*2+3];

    WaitForSingleObject(mtx_set, INFINITE);
    memset(tbuf, '.', INFO_LEN);
    strncpy(tbuf, ri->prefix, strlen(ri->prefix));
    tbuf[INFO_LEN] = ':';
    tbuf[INFO_LEN+1] = ' ';
    memset((char *)(tbuf+INFO_LEN+2), '\0', INFO_LEN+1);
    memcpy((char *)(tbuf+INFO_LEN+2), text, INFO_LEN);
    radarInfoSet(ri->__internal, tbuf, HRADAR_FONTSIZ, RC_YELLOW);
    ReleaseMutex(mtx_set);
}

extern void radarSetInfoF(struct radarInfo *ri, const char *fmt, ...)
{
    char buf[INFO_LEN+1];
    va_list argp;

    memset(buf, '\0', INFO_LEN+1);
    va_start(argp, fmt);
    vsnprintf(buf, INFO_LEN, fmt, argp);
    radarSetInfo(ri, buf);
    va_end(argp);
}

extern void radarSetDrawDistance(FLOAT vecDist)
{
    radarUpdateResolution();
    WaitForSingleObject(mtx_set, INFINITE);
    f_vecDistance = vecDist;
    /*
     * (f_vecDistance*2.0f): "absolute" distance from the left window border to the right
     */
    f_propX = gr_width/(f_vecDistance*2.0f); // calculate the proportional distance
    f_propY = gr_height/(f_vecDistance*2.0f);
    ReleaseMutex(mtx_set);
}

extern void radarSetPlayerPosition(FLOAT ppos[3], FLOAT angle)
{
    WaitForSingleObject(mtx_set, INFINITE);
    f_angle = angle;
    memcpy(f_playerPos, ppos, sizeof(FLOAT)*3);
    ReleaseMutex(mtx_set);
}

static inline void radarCalcPosition(sfVector2f *ptr_pos, FLOAT pos3f[3])
{
    sfVector2f pos;
    FLOAT f_entPlayerDiff[2];

    f_entPlayerDiff[0] = (pos3f[0] - f_playerPos[0]) + f_vecDistance;
    f_entPlayerDiff[1] = (pos3f[2] - f_playerPos[2]) + f_vecDistance;
    pos.x = f_entPlayerDiff[0] * f_propX - ENTITY_CIRCLE_RADIUS;
    pos.y = f_entPlayerDiff[1] * f_propY - ENTITY_CIRCLE_RADIUS;
    memcpy(ptr_pos, &pos, sizeof(sfVector2f));
}

extern void radarSetAimLine(FLOAT enemy_pos3f[3], BOOL enable)
{
    sfVector2f v1, v2, tmp;
    FLOAT a, b, c, angl;

    memset(&v1, '\0', sizeof(sfVector2f));
    memset(&v2, '\0', sizeof(sfVector2f));
    if (enable == FALSE)
    {
        sfRectangleShape_setPosition(aimLine, v1);
        sfRectangleShape_setSize(aimLine, v2);
    }
    else
    {
        a = (f_playerPos[0] - enemy_pos3f[0]) * f_propX;
        b = (f_playerPos[2] - enemy_pos3f[2]) * f_propY;
        c = sqrtf( powf(a, 2.0f) + powf(b, 2.0f) );
        if (a < 0)
        {
            angl = 360.0f/(2.0f*M_PI)*atanf( b/a ) + 270.0f;
        }
        else angl = 360.0f/(2.0f*M_PI)*atanf( b/a ) + 90.0f;
        v1.x = gr_width/2;
        v1.y = gr_height/2;
        v2.x = AIMLINE_WIDTH;
        v2.y = c;
        radarCalcPosition(&tmp, enemy_pos3f);
        sfRectangleShape_setPosition(aimLine, v1);
        sfRectangleShape_setSize(aimLine, v2);
        sfRectangleShape_setRotation(aimLine, angl);
    }
}

static void radarUpdateGraphicEnt(struct graphicEnt *ge, FLOAT f_vecPos3f[3], enum radarColor color)
{
    sfVector2f pos;

    /*
        printf("--- %8.2f , %8.2f ---\n", f_vecPos3f[0], f_vecPos3f[2]);
        if (abs(f_vecPos3f[0]) > f_vecDistance)
        {
            f_vecPos3f[0] = f_playerPos[0] + f_vecDistance * (f_vecPos3f[0] > 0 ? 1.0f : -1.0f);
        }
        if (abs(f_vecPos3f[2]) > f_vecDistance)
        {
            f_vecPos3f[2] = f_playerPos[2] + f_vecDistance * (f_vecPos3f[2] > 0 ? 1.0f : -1.0f);
        }
        printf("### %8.2f , %8.2f ---\n", f_vecPos3f[0], f_vecPos3f[2]);
    */
    radarCalcPosition(&pos, f_vecPos3f);
    sfCircleShape_setPosition(ge->cr_entShape, pos);
    sfCircleShape_setFillColor(ge->cr_entShape, radarSFMLColor(color));
}

extern void radarUpdateEntity(UINT64 unique_id, FLOAT pos[3], enum radarColor color, BOOL isPlayer, BOOL valid)
{
    WaitForSingleObject(mtx_set, INFINITE);
    struct graphicEnt *ge;
    struct radarEntity *re = rad_entityList, *prev = rad_entityList;

    if (color == RC_DONTDRAW) goto FIN;
    if (re == NULL) goto FIN;
    while ( (re = re->next) != NULL )
    {
        if (re->id == unique_id)
        {
            re->valid = valid;
            re->isPlayer = isPlayer;
            ge = (struct graphicEnt *) re->__internal;
            radarUpdateGraphicEnt(ge, pos, color);
            goto FIN;
        }
        prev = re;
    }
    prev->next = radarNewEntity(unique_id, isPlayer, TRUE);
    ge = (struct graphicEnt *) prev->next->__internal;
    radarUpdateGraphicEnt(ge, pos, color);
FIN:
    ReleaseMutex(mtx_set);
}

extern void radarInvalidateAll(void)
{
    WaitForSingleObject(mtx_set, INFINITE);
    struct radarEntity *re = rad_entityList;

    if (re == NULL) goto FIN;
    while ( (re = re->next) != NULL )
    {
        re->valid = FALSE;
    }
FIN:
    ReleaseMutex(mtx_set);
}

extern void radarRemoveInvalidEntities(void)
{
    WaitForSingleObject(mtx_set, INFINITE);
    struct radarEntity *re = rad_entityList, *prev = rad_entityList;

    if (re != NULL)
    {
        while ( (re = re->next) != NULL )
        {
            if (re->valid != TRUE)
            {
                prev->next = re->next;
                if (re->next != NULL)
                {
                    re->next->prev = prev;
                }
                radarDeleteEntity(re);
                re = prev;
            }
            else
            {
                prev = re;
            }
        }
    }
    ReleaseMutex(mtx_set);
}

extern void radarDrawEntities(void)
{
    struct graphicEnt *ge;
    struct radarEntity *re = rad_entityList;

    if (re == NULL) return;
    // draw everything except for players
    while ( (re = re->next) != NULL )
    {
        ge = (struct graphicEnt *) re->__internal;
        if (ge != NULL && re->isPlayer == FALSE)
        {
            sfRenderWindow_drawCircleShape(rWnd, ge->cr_entShape, NULL);
        }
    }
    re = rad_entityList;
    // draw players
    while ( (re = re->next) != NULL )
    {
        ge = (struct graphicEnt *) re->__internal;
        if (ge != NULL && re->isPlayer == TRUE)
        {
            sfRenderWindow_drawCircleShape(rWnd, ge->cr_entShape, NULL);
        }
    }
}

extern void radarExecThread(void)
{
    sThrd = sfThread_create(_radarLoop, NULL);
    if (sThrd != NULL)
    {
        printf("launching sfml thread ..\n");
        sfThread_launch(sThrd);
    }
}

extern void radarKillThread(void)
{
    sfThread_terminate(sThrd);
}

/* DLL entry point - windoze compat */
extern int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdParam, int cmdShow)
{
    return 0;
}
