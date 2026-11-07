#include "HostProcess.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>

namespace host
{
bool isRunning (unsigned long pid)
{
    HANDLE h = OpenProcess (SYNCHRONIZE, FALSE, pid);
    if (h == nullptr) return false;
    const bool alive = WaitForSingleObject (h, 0) == WAIT_TIMEOUT;
    CloseHandle (h);
    return alive;
}

static BOOL CALLBACK closeWindowsOf (HWND w, LPARAM pid)
{
    DWORD owner = 0;
    GetWindowThreadProcessId (w, &owner);
    if (owner == DWORD (pid) && IsWindowVisible (w) && GetWindow (w, GW_OWNER) == nullptr)
        PostMessageW (w, WM_CLOSE, 0, 0);
    return TRUE;
}

void requestClose (unsigned long pid) { EnumWindows (closeWindowsOf, LPARAM (pid)); }

int runElevatedAndWait (const std::wstring& exe, const std::wstring& args)
{
    SHELLEXECUTEINFOW info {};
    info.cbSize = sizeof (info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC;
    info.lpVerb = L"runas";
    info.lpFile = exe.c_str();
    info.lpParameters = args.c_str();
    info.nShow = SW_HIDE;
    if (! ShellExecuteExW (&info))
        return GetLastError() == ERROR_CANCELLED ? -2 : -1;
    if (info.hProcess == nullptr) return -1;
    WaitForSingleObject (info.hProcess, INFINITE);
    DWORD code = 1;
    GetExitCodeProcess (info.hProcess, &code);
    CloseHandle (info.hProcess);
    return int (code);
}

bool launch (const std::wstring& exe)
{
    return reinterpret_cast<INT_PTR> (ShellExecuteW (nullptr, L"open", exe.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) > 32;
}
} // namespace host
