#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "native.h"


typedef LONG (NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG (NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);

/* Standard error macro for reporting API errors */
#define PERR(bSuccess, api){if(!(bSuccess)) printf("%s:Error %ld from %s \
    on line %ld\n", __FILE__, GetLastError(), api, (long)__LINE__);}

void initNativeData(native_data *nd)
{
    assert(nd);
    nd->alloc_fn = mem_alloc;
    nd->read_fn = read_procmem;
    nd->write_fn = write_procmem;
    nd->suspend_fn = suspendProcess;
    nd->iterate_fn = iterate_mem;
}

/* see: https://support.microsoft.com/en-us/help/99261/how-to-performing-clear-screen-cls-in-a-console-application */
void cls(HANDLE hConsole)
{
    COORD coordScreen = { 0, 0 };    /* here's where we'll home the
                                        cursor */
    BOOL bSuccess;
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
    DWORD dwConSize;                 /* number of character cells in
                                        the current buffer */

    /* get the number of character cells in the current buffer */

    bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
    PERR( bSuccess, "GetConsoleScreenBufferInfo" );
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    /* fill the entire screen with blanks */

    bSuccess = FillConsoleOutputCharacter( hConsole, (TCHAR) ' ',
                                           dwConSize, coordScreen, &cCharsWritten );
    PERR( bSuccess, "FillConsoleOutputCharacter" );

    /* get the current text attribute */

    bSuccess = GetConsoleScreenBufferInfo( hConsole, &csbi );
    PERR( bSuccess, "ConsoleScreenBufferInfo" );

    /* now set the buffer's attributes accordingly */

    bSuccess = FillConsoleOutputAttribute( hConsole, csbi.wAttributes,
                                           dwConSize, coordScreen, &cCharsWritten );
    PERR( bSuccess, "FillConsoleOutputAttribute" );

    /* put the cursor at (0, 0) */

    bSuccess = SetConsoleCursorPosition( hConsole, coordScreen );
    PERR( bSuccess, "SetConsoleCursorPosition" );
    return;
}

bool get_module_proc(native_data *nd, LPCTSTR window_name)
{
    HWND hwnd;

    assert(window_name);
    hwnd = FindWindow(NULL, window_name);
    if (!hwnd)
        goto error;
    GetWindowThreadProcessId(hwnd, &nd->proc.pid);
    if (!nd->proc.pid)
        goto error;
    nd->proc.hndl = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION
                                | PROCESS_VM_READ | PROCESS_VM_WRITE, 0, nd->proc.pid);
error:
    return nd->proc.hndl != NULL;
}

bool get_module_info(native_data *nd, LPCTSTR module_name)
{
    HMODULE hMods[1024];
    DWORD cbNeeded;
    unsigned int i;

    assert(module_name);
    if (EnumProcessModules(nd->proc.hndl, hMods, sizeof(hMods), &cbNeeded))
    {
        for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];
            TCHAR szModPath[MAX_PATH];

            if (GetModuleBaseName(nd->proc.hndl, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))
                    && GetModuleFileNameEx(nd->proc.hndl, hMods[i], szModPath,
                                           sizeof(szModPath) / sizeof(TCHAR)))
            {
                if (strncmp(szModName, module_name, MAX_PATH) == 0)
                {
                    nd->proc.modbase = hMods[i];
                    MODULEINFO modinfo;
                    if (GetModuleInformation(nd->proc.hndl, nd->proc.modbase, &modinfo, sizeof modinfo))
                        nd->proc.modsize = modinfo.SizeOfImage;
                    else
                        return false;

                    return true;
                }
            }
        }
    }
    return false;
}

bool read_procmem(const native_data *nd, unsigned long addr,
                  void *buffer, unsigned long siz)
{
    SIZE_T bytes_read = 0;
    unsigned long *vmptr = (unsigned long *)addr;

    assert(buffer && siz);
    if (!ReadProcessMemory(nd->proc.hndl, vmptr, buffer, siz, &bytes_read))
        return false;
    if (bytes_read != siz)
        return false;

    return true;
}

bool write_procmem(const native_data *nd, unsigned long addr, const void *buffer, unsigned long siz)
{
    SIZE_T bytes_written = 0;
    unsigned long *vmptr = (unsigned long *)addr;

    assert(addr && buffer && siz);
    if (!WriteProcessMemory(nd->proc.hndl, vmptr, buffer, siz, &bytes_written))
        return false;
    if (bytes_written != siz)
        return false;

    return true;
}

unsigned long mem_alloc(const native_data *nd, unsigned long siz)
{
    return (unsigned long)VirtualAllocEx(nd->proc.hndl, NULL, siz,
                                         MEM_COMMIT | MEM_RESERVE,
                                         PAGE_EXECUTE_READWRITE);
}

/* see: https://github.com/mridgers/clink/issues/420 */
bool suspendProcess(const native_data *nd, int doResume)
{
    bool ret = false;
    NtSuspendProcess pfnNtSuspendProcess =
        (NtSuspendProcess)GetProcAddress(GetModuleHandle("ntdll"), "NtSuspendProcess");
    NtResumeProcess pfnNtResumeProcess =
        (NtResumeProcess)GetProcAddress(GetModuleHandle("ntdll"), "NtResumeProcess");

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, nd->proc.pid);
    if (!processHandle)
        return false;
    if (doResume)
    {
        if (pfnNtResumeProcess(processHandle) == 0)
            ret = true;
    }
    else
    {
        if (pfnNtSuspendProcess(processHandle) == 0)
            ret = true;
    }
    CloseHandle(processHandle);
    return ret;
}

unsigned long iterate_mem(const native_data *nd,
                          unsigned long *addr,
                          unsigned long *size)
{
    MEMORY_BASIC_INFORMATION info;
    SIZE_T ret;

    ret = VirtualQueryEx(nd->proc.hndl, (LPCVOID) *addr, &info, sizeof info);
    if (ret != sizeof(info))
        goto error;
    *addr = (unsigned long) info.BaseAddress;
    *size = info.RegionSize;
    return info.State != MEM_COMMIT;
error:
    *addr = 0;
    *size = 0;
    return 1;
}
