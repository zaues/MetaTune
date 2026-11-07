#pragma once
#include <string>

// Small Win32 layer for the updater (kept out of the JUCE translation units).
namespace host
{
bool isRunning (unsigned long pid);
// Asks the process to close the way clicking its window's X does (WM_CLOSE to its top-level windows),
// so FL Studio still offers to save the project. Never terminates it.
void requestClose (unsigned long pid);
// Runs an installer elevated (one UAC prompt) and waits for it. Returns its exit code,
// -2 if the UAC prompt was declined, -1 if it could not start.
int runElevatedAndWait (const std::wstring& exe, const std::wstring& args);
// Starts a program normally (reopening the host).
bool launch (const std::wstring& exe);
} // namespace host
