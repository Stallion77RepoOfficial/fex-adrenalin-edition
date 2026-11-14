// SPDX-License-Identifier: MIT
// Build as a PE32 executable with --disable-nxcompat. Run "legacy" and "enforce"
// through Wine/WOW64: both must return zero. No host execution policy is changed.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

static int ExpectFault;

static LONG CALLBACK HandleFault(EXCEPTION_POINTERS *Exception) {
  if (Exception->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION &&
      Exception->ExceptionRecord->NumberParameters >= 2 && Exception->ExceptionRecord->ExceptionInformation[0] == 8) {
    puts(ExpectFault ? "PASS: DEP rejects a non-executable page" : "FAIL: legacy execution was rejected");
    fflush(stdout);
    ExitProcess(ExpectFault ? 0 : 42);
  }
  return EXCEPTION_CONTINUE_SEARCH;
}

int main(int argc, char **argv) {
  typedef LONG(WINAPI *SetInformationFn)(HANDLE, ULONG, void *, ULONG);
  static const unsigned char Program[] = {0xb8, 0x78, 0x56, 0x34, 0x12, 0xc3};
  ULONG Flags = 2; // MEM_EXECUTE_OPTION_ENABLE: legacy process, DEP disabled.
  SetInformationFn SetInformation = (SetInformationFn)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtSetInformationProcess");
  if (!SetInformation || SetInformation(GetCurrentProcess(), 34, &Flags, sizeof(Flags))) {
    puts("FAIL: cannot set legacy process policy");
    return 2;
  }
  AddVectoredExceptionHandler(1, HandleFault);
  unsigned char *Writable = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  unsigned char *Executable = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!Writable || !Executable) {
    return 3;
  }
  memcpy(Writable, Program, sizeof(Program));
  memcpy(Executable, Program, sizeof(Program));
  DWORD OldProtection;
  if (!VirtualProtect(Executable, 4096, PAGE_EXECUTE_READ, &OldProtection)) {
    return 4;
  }
  FlushInstructionCache(GetCurrentProcess(), NULL, 0);
  if (argc > 1 && !strcmp(argv[1], "enforce")) {
    if (!SetProcessDEPPolicy(PROCESS_DEP_ENABLE)) {
      printf("FAIL: cannot re-enable DEP: %lu\n", GetLastError());
      return 5;
    }
  }
  // DLL code and a page promoted from RW to RX must remain executable.
  if (((unsigned int (*)(void))Executable)() != 0x12345678) {
    return 6;
  }
  puts("PASS: explicitly executable code survives the policy transition");
  fflush(stdout);
  ExpectFault = argc > 1 && !strcmp(argv[1], "enforce");
  if (((unsigned int (*)(void))Writable)() != 0x12345678) {
    return 7;
  }
  if (ExpectFault) {
    puts("FAIL: DEP allowed a non-executable page");
    return 8;
  }
  puts("PASS: legacy process can execute a readable page");
  VirtualFree(Writable, 0, MEM_RELEASE);
  VirtualFree(Executable, 0, MEM_RELEASE);
  return 0;
}
