//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// TEMPORARY CI DIAGNOSTIC -- not for merge.
// Dumps native stacks of every thread when a single test exceeds a deadline,
// so a CI-only hang can be root-caused without a debugger on the runner.
//
#ifdef _WIN32

#include <windows.h>
#include <dbghelp.h>
#include <tlhelp32.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

#pragma comment(lib, "dbghelp.lib")

namespace {

std::atomic<long long> g_testStartMs{0};

long long NowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

void DumpAllThreadStacks()
{
    HANDLE proc = ::GetCurrentProcess();
    ::SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES);
    char exeDir[MAX_PATH] = {0};
    ::GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash != nullptr)
    {
        *lastSlash = '\0';
    }
    ::SymInitialize(proc, exeDir, TRUE);

    DWORD pid = ::GetCurrentProcessId();
    DWORD selfTid = ::GetCurrentThreadId();
    HANDLE snap = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE)
    {
        printf("watchdog: CreateToolhelp32Snapshot failed %lu\n", ::GetLastError());
        return;
    }

    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (!::Thread32First(snap, &te))
    {
        ::CloseHandle(snap);
        return;
    }

    do
    {
        if (te.th32OwnerProcessID != pid || te.th32ThreadID == selfTid)
        {
            continue;
        }

        HANDLE th = ::OpenThread(
            THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME,
            FALSE, te.th32ThreadID);
        if (th == NULL)
        {
            continue;
        }

        std::vector<DWORD64> frames;
        ::SuspendThread(th);
        CONTEXT ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.ContextFlags = CONTEXT_FULL;
        if (::GetThreadContext(th, &ctx))
        {
            STACKFRAME64 f;
            memset(&f, 0, sizeof(f));
#ifdef _M_IX86
            DWORD machine = IMAGE_FILE_MACHINE_I386;
            f.AddrPC.Offset = ctx.Eip;
            f.AddrFrame.Offset = ctx.Ebp;
            f.AddrStack.Offset = ctx.Esp;
#else
            DWORD machine = IMAGE_FILE_MACHINE_AMD64;
            f.AddrPC.Offset = ctx.Rip;
            f.AddrFrame.Offset = ctx.Rbp;
            f.AddrStack.Offset = ctx.Rsp;
#endif
            f.AddrPC.Mode = AddrModeFlat;
            f.AddrFrame.Mode = AddrModeFlat;
            f.AddrStack.Mode = AddrModeFlat;

            for (int i = 0; i < 64; i++)
            {
                if (!::StackWalk64(machine, proc, th, &f, &ctx, NULL,
                        ::SymFunctionTableAccess64, ::SymGetModuleBase64, NULL))
                {
                    break;
                }
                if (f.AddrPC.Offset == 0)
                {
                    break;
                }
                frames.push_back(f.AddrPC.Offset);
            }
        }
        ::ResumeThread(th);
        ::CloseHandle(th);

        printf("\n--- thread %lu (%zu frames) ---\n",
            static_cast<unsigned long>(te.th32ThreadID), frames.size());
        for (size_t i = 0; i < frames.size(); i++)
        {
            alignas(8) char buf[sizeof(SYMBOL_INFO) + 512];
            memset(buf, 0, sizeof(buf));
            SYMBOL_INFO* sym = reinterpret_cast<SYMBOL_INFO*>(buf);
            sym->SizeOfStruct = sizeof(SYMBOL_INFO);
            sym->MaxNameLen = 500;

            char modName[MAX_PATH] = "?";
            DWORD64 modBase = 0;
            IMAGEHLP_MODULE64 mi;
            memset(&mi, 0, sizeof(mi));
            mi.SizeOfStruct = sizeof(mi);
            int symType = -1;
            if (::SymGetModuleInfo64(proc, frames[i], &mi))
            {
                strncpy_s(modName, mi.ModuleName, _TRUNCATE);
                modBase = mi.BaseOfImage;
                symType = static_cast<int>(mi.SymType);
            }
            unsigned long long rva = (modBase != 0)
                ? static_cast<unsigned long long>(frames[i] - modBase) : 0ull;

            DWORD64 disp = 0;
            if (::SymFromAddr(proc, frames[i], &disp, sym))
            {
                printf("  %02zu %s+0x%llx  %s!%s+0x%llx\n", i, modName, rva, modName, sym->Name,
                    static_cast<unsigned long long>(disp));
            }
            else
            {
                printf("  %02zu %s+0x%llx  (symtype=%d)\n", i, modName, rva, symType);
            }
        }
        fflush(stdout);
    } while (::Thread32Next(snap, &te));

    ::CloseHandle(snap);
}

}  // namespace

void FuncTestsWatchdog_NoteTestStart()
{
    g_testStartMs.store(NowMs());
}

void FuncTestsWatchdog_Start()
{
    const char* env = nullptr;
    size_t len = 0;
    char buffer[32] = {0};
    if (::getenv_s(&len, buffer, sizeof(buffer), "FUNCTESTS_WATCHDOG_SEC") == 0 && len > 1)
    {
        env = buffer;
    }
    unsigned seconds = (env != nullptr) ? static_cast<unsigned>(atoi(env)) : 0u;
    if (seconds == 0)
    {
        return;
    }

    g_testStartMs.store(NowMs());
    std::thread([seconds]() {
        for (;;)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            long long started = g_testStartMs.load();
            if (started == 0)
            {
                continue;
            }
            if ((NowMs() - started) > static_cast<long long>(seconds) * 1000)
            {
                printf("\n#### WATCHDOG: current test exceeded %u s -- dump #1 ####\n", seconds);
                fflush(stdout);
                DumpAllThreadStacks();
                std::this_thread::sleep_for(std::chrono::seconds(45));
                printf("\n#### WATCHDOG: dump #2 (45 s later; identical stacks == truly stuck) ####\n");
                fflush(stdout);
                DumpAllThreadStacks();
                printf("\n#### WATCHDOG: terminating process ####\n");
                fflush(stdout);
                ::TerminateProcess(::GetCurrentProcess(), 99);
            }
        }
    }).detach();
}

#else
void FuncTestsWatchdog_NoteTestStart() {}
void FuncTestsWatchdog_Start() {}
#endif
