#ifndef MEM_H_INCLUDED
#define MEM_H_INCLUDED

#include <stdio.h>
#include <wchar.h>
#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>


#define readByte (adr, b_data)   readMem(hndl_proc, adr, (LPVOID) &b_data, sizeof(BYTE))
#define readInt32(adr, d_data)   readMem(hndl_proc, adr, (LPVOID) &d_data, sizeof(INT32))
#define readInt64(adr, q_data)   readMem(hndl_proc, adr, (LPVOID) &q_data, sizeof(INT64))
#define readFloat(adr, f_data)   readMem(hndl_proc, adr, (LPVOID) &f_data, sizeof(FLOAT))
#define readMemH(adr, data, siz) readMem(hndl_proc, adr, data, siz)
#define readPtrH(adr, offset)    readPtr(hndl_proc, adr, offset)

extern LPCVOID calcOffset(LPCVOID adr, SIZE_T offset);

extern SIZE_T readMem(HANDLE hProcess, LPCVOID adr, LPVOID data, SIZE_T siz);

extern LPCVOID readPtr(HANDLE hProcess, LPCVOID adr, SIZE_T offset);

extern SIZE_T findPattern(BYTE *b, const BYTE *p, const char *mask, SIZE_T b_siz, SIZE_T pm_siz);

extern LPCVOID getBaseAdr(BYTE *buf, SIZE_T siz, const BYTE *pat, const char *mask, const SIZE_T pm_siz, const SIZE_T off_pos);

#endif // MEM_H_INCLUDED
