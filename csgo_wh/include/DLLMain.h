#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>
#include "D9DW.h"

#define VERSION "0.3a"
#define COPYRIGHT "CS:GO_HACK (C) by lnslbrty"
#define CINTERFACE 1

#ifdef BUILD_DLL
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C"
{
#endif

__declspec(dllexport) BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

#ifdef ENABLE_DEBUG
#define DBG(fmt, ...) D9DW::DbgMessageBox(256, fmt, __VA_ARGS__)
void DbgMessageBox(size_t sz_len, const char *fmt, ...);
#else
#define DBG(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
