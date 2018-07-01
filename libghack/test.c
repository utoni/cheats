#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>
#include <ghack.h>

static void log_test(void)
{
    log_init("TEST_PROG");
    dbg("%s", L"hello_log");
    log_to(__FILE__, __LINE__, "%s", "an ASCII log string");
    logw_to(TEXT(__FILE__), __LINE__, L"%s", L"a WIDE log string");
    log_flush();
    log_close();
}

int main(int argc, char **argv)
{
    unsigned int vDist = 3000.0f;
    int i = 150;
    FLOAT ppos[3] = { -1200.0f, -117.0f, 0.0f };
    FLOAT epos_01[3] = { -1200.0f, 784.0f, 35.0f };
    FLOAT epos_02[3] = { 525.0f, 445.0f, 55.0f };
    FLOAT epos_03[3] = { 325.0f, 45.0f, 24.0f };
    struct radarConfig rad_cfg;
    struct radarInfo *nfo_ver;

    log_test();

    memset(&rad_cfg, '\0', sizeof(rad_cfg));
    strncpy(rad_cfg.wnd_name, "TESTWND", RDR_NAMELEN);
    radarInit(&rad_cfg);
    radarSetDrawDistance(vDist);
    printf("MaxDist: %u | Width: %u | Height: %u | PropX: %+8.2f | PropY: %+8.2f\n", vDist, radarGetWidth(), radarGetHeight(), radarPropX(), radarPropY());
    radarExecThread();
    radarWaitUntilRdy();
    nfo_ver = radarAddInfo("Test");
    radarSetInfoF(nfo_ver, "%d", i);
    radarSetPlayerPosition(ppos, 120.0f);
    radarUpdateEntity(0x001, epos_01, RC_GREEN, FALSE, TRUE);
    radarUpdateEntity(0x002, epos_02, RC_YELLOW, FALSE, TRUE);
    radarUpdateEntity(0x003, epos_03, RC_BLUE, TRUE, TRUE);
    radarUpdateEntity(0x004, ppos, RC_RED, FALSE, TRUE);
    radarInvalidateAll();
    radarRemoveInvalidEntities();
    Sleep(1000);
    radarUpdateEntity(0x001, epos_01, RC_GREEN, FALSE, TRUE);
    radarUpdateEntity(0x002, epos_02, RC_YELLOW, FALSE, TRUE);
    radarUpdateEntity(0x003, epos_03, RC_BLUE, TRUE, TRUE);
    radarUpdateEntity(0x004, ppos, RC_RED, FALSE, TRUE);
    epos_03[0] = 250.0f;
    epos_03[2] = 125.0f;
    while (--i)
    {
        radarSetInfoF(nfo_ver, "%d", i);

        epos_03[0] -= 12.0f;
        epos_03[2] -= 3.5f;
        ppos[0] += 1.0f;
        radarUpdateEntity(0x001, epos_01, RC_GREEN, FALSE, TRUE);
        radarUpdateEntity(0x002, epos_02, RC_YELLOW, FALSE, TRUE);
        radarUpdateEntity(0x003, epos_03, RC_BLUE, TRUE, TRUE);
        radarUpdateEntity(0x004, ppos, RC_RED, FALSE, TRUE);
        radarSetPlayerPosition(ppos, 45.0f + i);
        radarSetAimLine(epos_03, TRUE);

        Sleep(10);
    }
    radarUpdateEntity(0x004, ppos, RC_RED, FALSE, FALSE);
    radarRemoveInvalidEntities();
    epos_01[0] = vDist*-5.0f;
    epos_01[2] = vDist*-5.0f;
    epos_02[0] = ppos[0] - 225.0f;
    epos_02[2] = ppos[2] +
                 225.0f;
    epos_03[0] = 0.0f;
    epos_03[2] = vDist;
    radarUpdateEntity(0x001, epos_01, RC_GREEN, FALSE, TRUE);
    radarUpdateEntity(0x002, epos_02, RC_YELLOW, FALSE, TRUE);
    radarUpdateEntity(0x003, epos_03, RC_YELLOW, FALSE, TRUE);
    radarSetAimLine(epos_02, TRUE);
    getchar();
    radarKillThread();
    radarCleanup();
    return 0;
}
