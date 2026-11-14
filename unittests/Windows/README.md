# WOW64 execution policy regression

Build `DEPPolicy.c` with an x86 Windows compiler and NX compatibility disabled:

```sh
i686-w64-mingw32-clang -O2 -Wall -Wextra -Wl,--disable-nxcompat DEPPolicy.c -o /tmp/DEPPolicy.exe
wine /tmp/DEPPolicy.exe legacy
wine /tmp/DEPPolicy.exe enforce
```

Both runs must return zero. The legacy run executes a readable page. The enforce
run keeps code changed from RW to RX executable, then catches an execution fault
on a page that remains RW. Run with a Wine build that forwards
`ProcessExecuteFlags` changes to `BTCpuNotifyProcessExecuteFlagsChange`.
