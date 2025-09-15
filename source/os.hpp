#pragma once

#include "types.hpp"

#define EXPORT

struct AppInfo
{
    u64 permMemorySize, tempMemorySize;
};

struct Metrics
{
    bool initialized;
    void *processHandle;

    Metrics();
    static Metrics &Get();

    u64 ReadPageFaultCount();
};

u64 ReadCPUTimer(void);

u64 EstimateCPUTimerFreq(void);

struct SystemInfo
{
    // System
    cstr processorArchitecture;
    u32 numberOfProcessors, pageSize, allocationGranularity;
    f64 cpuFreq;

    // Memory
    u64 totalPhys, availPhys, totalVirtual, availVirtual;

    // OS
    u32 majorVersion, minorVersion, buildNumber, platformId;

    // GPU
    cstr gpuName, gpuVendor, glVersion;

    static SystemInfo Init();

    void Print() const
    {
        INFO("System Information");
        printf("\t> Platform: \t\t\tWindows %s\n", processorArchitecture);
        printf("\t> Version: \t\t\t%u.%u.%u\n", majorVersion, minorVersion, buildNumber);
        printf("\t> Processor Count: \t\t%u\n", numberOfProcessors);
        printf("\t> CPU Frequency: \t\t%.2f GHz\n", cpuFreq);
        printf("\t> Page Size: \t\t\t%u bytes\n", pageSize);

        INFO("Memory Information");
        printf("\t> Total Physical Memory: \t%llu MB\n", totalPhys / (1024 * 1024));
        printf("\t> Available Physical Memory: \t%llu MB\n", availPhys / (1024 * 1024));
        printf("\t> Total Virtual Memory: \t%llu MB\n", totalVirtual / (1024 * 1024));
        printf("\t> Available Virtual Memory: \t%llu MB\n", availVirtual / (1024 * 1024));
    }
};
