#ifndef NATIVE_H_INCLUDED
#define NATIVE_H_INCLUDED

#include <windows.h>
#include <stdbool.h>

#define EXPORT __declspec(dllexport)

typedef struct native_data native_data;

typedef unsigned long(*alloc_mem_fn)(const native_data *nd,
                                     unsigned long siz);
typedef bool(*read_mem_fn)(const native_data *nd,
                           unsigned long addr, void *buffer,
                           unsigned long siz);
typedef bool(*write_mem_fn)(const native_data *nd,
                            unsigned long addr, const void *buffer,
                            unsigned long siz);
typedef bool(*suspend_proc_fn)(const native_data *nd,
                               int doResume);

typedef struct win_proc
{
    DWORD pid;
    HANDLE hndl;
    unsigned long modbase;
} win_proc;

typedef struct native_data
{
    win_proc proc;
    alloc_mem_fn alloc_fn;
    read_mem_fn read_fn;
    write_mem_fn write_fn;
    suspend_proc_fn suspend_fn;
} native_data;

EXPORT void initNativeData(native_data *nd);
EXPORT void cls(HANDLE hConsole);
EXPORT bool get_module_proc(native_data *nd,
                            LPCTSTR window_name);
EXPORT bool get_module_base(native_data *nd,
                            LPCTSTR module_name);

EXPORT unsigned long mem_alloc(const native_data *nd,
                               unsigned long siz);
EXPORT bool read_procmem(const native_data *nd,
                         unsigned long addr, void *buffer,
                         unsigned long siz);
EXPORT bool write_procmem(const native_data *nd,
                          unsigned long addr, const void *buffer,
                          unsigned long siz);
EXPORT bool suspendProcess(const native_data *nd,
                           int doResume);

#endif // NATIVE_H_INCLUDED
