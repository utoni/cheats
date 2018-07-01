#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#include <stdio.h>
#include <windows.h>
#ifdef UNICODE
#include <wchar.h>
#endif

extern FILE *log_file;


#ifdef UNICODE
#define dbg(msg, ...) logw_to(TEXT(__FILE__), __LINE__, TEXT(msg), __VA_ARGS__)
#else
#define dbg(msg, ...) log_to(__FILE__, __LINE__, msg, __VA_ARGS__)
#endif

extern BOOL log_init(char *p_fname);

extern void log_to(const char *file, SIZE_T line, const char *msg, ...);

#ifdef UNICODE
extern void logw_to(const char *file, SIZE_T line, char *msg, ...);
#endif

extern void log_flush(void);

extern void log_close(void);

#endif // LOG_H_INCLUDED
