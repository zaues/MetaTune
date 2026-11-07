#pragma once
#include <string>
#include <vector>

// Small Win32 layer for the updater (kept out of the JUCE translation units).
namespace host
{
bool isRunning (unsigned long pid);
// Asks the process to close the way clicking its window's X does (WM_CLOSE to its top-level windows),
// so FL Studio still offers to save the project. Never terminates it.
// Also lets the host take the foreground, so its save prompt can come up in front of the updater.
void requestClose (unsigned long pid);
// Visible top-level windows of a process (snapshot before closing, to spot the save prompt it opens).
std::vector<void*> windowsOf (unsigned long pid);
// Activates `theirs` and slots `ours` directly behind it (ours is not activated).
void raiseAbove (void* theirs, void* ours);
// Slots `ours` directly behind `theirs` again, without activating anything; false once `theirs` is gone.
bool keepBehind (void* theirs, void* ours);
// Runs an installer elevated (one UAC prompt) and waits for it. Returns its exit code,
// -2 if the UAC prompt was declined, -1 if it could not start.
int runElevatedAndWait (const std::wstring& exe, const std::wstring& args);
// Starts a program normally (reopening the host).
bool launch (const std::wstring& exe);
} // namespace host
