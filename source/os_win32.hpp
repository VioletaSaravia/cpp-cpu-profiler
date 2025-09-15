#include "os.hpp"

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS 1

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#pragma clang diagnostic ignored "-Wextra"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <Windows.h>

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <Psapi.h>

#undef EXPORT
#define EXPORT extern "C" __declspec(dllexport)

Metrics::Metrics() : initialized{true}, processHandle{OpenProcess(
                                            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId())} {}

u64 Metrics::ReadPageFaultCount()
{
    PROCESS_MEMORY_COUNTERS_EX counters = {};

    counters.cb = sizeof(counters);
    GetProcessMemoryInfo(processHandle, (PROCESS_MEMORY_COUNTERS *)&counters, sizeof(counters));

    u64 result = counters.PageFaultCount;
    return result;
}

u64 EstimateCPUTimerFreq(void)
{
    u64 MillisecondsToWait = 100;

    LARGE_INTEGER OSFreq;
    QueryPerformanceFrequency(&OSFreq);

    u64 CPUStart = ReadCPUTimer();
    LARGE_INTEGER OSStart;
    QueryPerformanceCounter(&OSStart);
    LARGE_INTEGER OSEnd;
    u64 OSElapsed = 0;
    u64 OSWaitTime = OSFreq.QuadPart * MillisecondsToWait / 1000;
    while (OSElapsed < OSWaitTime)
    {
        QueryPerformanceCounter(&OSEnd);
        OSElapsed = OSEnd.QuadPart - OSStart.QuadPart;
    }

    u64 CPUEnd = ReadCPUTimer();
    u64 CPUElapsed = CPUEnd - CPUStart;

    u64 CPUFreq = 0;
    if (OSElapsed)
    {
        CPUFreq = OSFreq.QuadPart * CPUElapsed / OSElapsed;
    }

    return CPUFreq;
};

u64 ReadCPUTimer(void)
{
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64) || \
    defined(__i386__) || defined(_M_IX86)
    return __rdtsc();

#elif defined(__aarch64__)
    // ARMv8 (AArch64): use CNTVCT_EL0
    uint64_t cnt;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(cnt));
    return cnt;

#elif defined(__arm__)
    // ARMv7-A: use PMCCNTR (if enabled)
    uint32_t cc;
    __asm__ volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(cc));
    return (uint64_t)cc;

#else
#error "Unsupported architecture"
#endif
}

SystemInfo SystemInfo::Init()
{
    SystemInfo result = {};
    SYSTEM_INFO sysInfo;
    MEMORYSTATUSEX memInfo;
    OSVERSIONINFOEX osInfo;

    // System
    GetSystemInfo(&sysInfo);
    // processorArchitecture = sysInfo.wProcessorArchitecture;
    result.numberOfProcessors = sysInfo.dwNumberOfProcessors;
    result.pageSize = sysInfo.dwPageSize;
    result.allocationGranularity = sysInfo.dwAllocationGranularity;

    result.processorArchitecture = "Unknown";
    switch (sysInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        result.processorArchitecture = "x64 (AMD/Intel)";
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        result.processorArchitecture = "x86";
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        result.processorArchitecture = "ARM";
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        result.processorArchitecture = "ARM64";
        break;
    }

    result.cpuFreq = f64(EstimateCPUTimerFreq()) / 1000.0 / 1000.0 / 1000.0;

    // Memory
    memInfo.dwLength = sizeof(memInfo);
    if (GlobalMemoryStatusEx(&memInfo))
    {
        result.totalPhys = memInfo.ullTotalPhys;
        result.availPhys = memInfo.ullAvailPhys;
        result.totalVirtual = memInfo.ullTotalVirtual;
        result.availVirtual = memInfo.ullAvailVirtual;
    }
    else
    {
        result.totalPhys = result.availPhys = result.totalVirtual = result.availVirtual = 0;
    }

    // OS
    ZeroMemory(&osInfo, sizeof(osInfo));
    osInfo.dwOSVersionInfoSize = sizeof(osInfo);
    if (GetVersionEx((OSVERSIONINFO *)&osInfo))
    {
        result.majorVersion = osInfo.dwMajorVersion;
        result.minorVersion = osInfo.dwMinorVersion;
        result.buildNumber = osInfo.dwBuildNumber;
        result.platformId = osInfo.dwPlatformId;
    }
    else
    {
        result.majorVersion = result.minorVersion = result.buildNumber = result.platformId = 0;
    }

    result.Print();

    return result;
}