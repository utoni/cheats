#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "log.h"

FILE *log_file = NULL;
static time_t log_initTime;


extern BOOL log_init(char *p_fname)
{
    char fpath[BUFSIZ];

    memset(&fpath[0], '\0', BUFSIZ);
    snprintf(fpath, BUFSIZ, "%s.log", p_fname);
    log_initTime = time(NULL);
    return ( (log_file = fopen(fpath, "a+")) != NULL );
}

extern void log_to(const char *file, SIZE_T line, const char *msg, ...)
{
    time_t now;
    double diffTime;
    char buf[BUFSIZ+1];
    char out[BUFSIZ+1];
    va_list va;
    va_start(va, msg);

    memset(&buf[0], '\0', (BUFSIZ+1)*sizeof(char));
    memset(&out[0], '\0', (BUFSIZ+1)*sizeof(char));
    vsnprintf(buf, BUFSIZ, msg, va);
    now = time(NULL);
    diffTime = difftime(now, log_initTime);
    snprintf(out, BUFSIZ, "%8.0f [%s:%lu]: %s\n", diffTime, file, line, buf);
    if (log_file != NULL)
    {
        fprintf(log_file, "%s", out);
    }
    else
    {
        printf("%s", out);
    }
}

#ifdef UNICODE
extern void logw_to(const char *file, SIZE_T line, char *msg, ...)
{
    if (!log_file) return;
    time_t now;
    double diffTime;
    wchar_t buf[BUFSIZ+1];
    wchar_t out[BUFSIZ+1];
    va_list va;
    va_start(va, msg);

    memset(&buf[0], '\0', (BUFSIZ+1)*sizeof(char));
    memset(&out[0], '\0', (BUFSIZ+1)*sizeof(char));
    vsnwprintf(buf, BUFSIZ, msg, va);
    now = time(NULL);
    diffTime = difftime(now, log_initTime);
    snwprintf(out, BUFSIZ, L"%8.0f [%s:%lu]: %s\n", diffTime, file, line, buf);
    fwprintf(log_file, L"%s", out);
}
#endif

extern void log_flush(void)
{
    if (log_file)
    {
        fflush(log_file);
    }
}

extern void log_close(void)
{
    if (!log_file) return;
    log_to(__FILE__, __LINE__, "%s", "closing log file");
    log_flush();
    fclose(log_file);
    log_file = NULL;
}
