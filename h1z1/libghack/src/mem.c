#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "log.h"
#include "mem.h"


extern LPCVOID calcOffset(LPCVOID adr, SIZE_T offset)
{
    LPCVOID r_adr = (LPCVOID)( (LPCVOID)adr+(SIZE_T)offset );
    return r_adr;
}

extern SIZE_T readMem(HANDLE hProcess, LPCVOID adr, LPVOID data, SIZE_T siz)
{
    SIZE_T rpmBytesRead = 0;
    if ( ReadProcessMemory( hProcess, adr, data, siz, &rpmBytesRead ) == FALSE )
    {
        log_to(__FILE__, __LINE__, "Could not read memory @ 0x%p (%lu/%lu bytes)", adr, siz, rpmBytesRead);
        return 0;
    }
    return rpmBytesRead;
}

extern LPCVOID readPtr(HANDLE hProcess, LPCVOID adr, SIZE_T offset)
{
    LPCVOID r_adr = calcOffset( adr, offset );
    LPCVOID ptr;
    if (readMem(hProcess, r_adr, &ptr, sizeof(LPCVOID)) > 0)
    {
        return ptr;
    }
    log_to(__FILE__, __LINE__, "Could not read pointer @ 0x%p (ERROR: %d)", r_adr, GetLastError());
    return NULL;
}

extern SIZE_T findPattern(BYTE *b, const BYTE *p, const char *mask, SIZE_T b_siz, SIZE_T pm_siz)
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
            if ( *(char *)(mask + j - 1) == '?' )
            {
                return (i - pm_siz + 1);
            }
            else j = 0;
        }
    }
    return i;
}

extern LPCVOID getBaseAdr(BYTE *buf, SIZE_T siz, const BYTE *pat, const char *mask, const SIZE_T pm_siz, const SIZE_T off_pos)
{
    SIZE_T pos = 0;

    if ( (pos = findPattern(&buf[0], &pat[0], &mask[0], siz, pm_siz)) <= (siz - pm_siz) )
    {
        pos += off_pos;
    }
    else
    {
        log_to(__FILE__, __LINE__, "Could not get base adress (mask: %s)", mask);
        return 0x0;
    }
    return ( (LPCVOID) pos );
}
