#if defined(_WIN32)
#include "os_win32.hpp"
#elif defined(__linux__)
#include "os_linux.hpp"
#elif defined(__EMSCRIPTEN__)
#include "os_wasm.hpp"
#else
#error "Unsupported platform"
#endif

#include "types.hpp"
#include "containers.hpp"

struct Block
{
    cstr label, file;
    i32 line;

    u64 iterations;
    u64 from, timeEx, timeInc;

    u64 bytesProcessed;
};

#ifndef MAX_BLOCKS
#define MAX_BLOCKS 64
#endif

struct Profiler
{
    struct BlockFlag
    {
        Profiler *parent;
        ~BlockFlag() { parent->EndBlock(); }
    };

    cstr name;
    bool ended;
    u64 start;

    StackArray<Block, MAX_BLOCKS> blocks;
    StackArray<u64, MAX_BLOCKS> queue;

    static Profiler _Profiler;
    static bool Initialized; // Prevents destructor from being called on init <.<
    static Profiler &Get() { return Profiler::_Profiler; }

    Profiler(cstr _name = "") : name{_name}, ended{false}, start{0}, blocks{}, queue{}
    {
        LARGE_INTEGER perfCounter;
        QueryPerformanceCounter(&perfCounter);
        start = perfCounter.QuadPart;
    }

    void BeginBlock(u64 id, cstr label = "", cstr file = "", i32 line = 0, u64 bytesProcessed = 0)
    {
        if (id >= blocks.cap)
        {
            return;
        }

        Block *m = &blocks[id];
        LARGE_INTEGER time;
        QueryPerformanceCounter(&time);

        if (queue.len > 0)
        {
            Block *prev = &blocks[queue.Last()];
            prev->timeEx += time.QuadPart - prev->from;
            prev->timeInc += time.QuadPart - prev->from;
        }

        m->from = time.QuadPart;
        m->label = label;
        m->file = file;
        m->line = line;
        m->bytesProcessed += bytesProcessed;

        queue.Push(id);

        m->iterations++;

        return;
    }

    void AddBytes(u64 bytes) { blocks[queue.Last()].bytesProcessed += bytes; }

    BlockFlag
    BeginScopeBlock(i32 id, cstr label, cstr file = "", i32 line = 0, u64 bytesProcessed = 0)
    {
        BeginBlock(id, label, file, line, bytesProcessed);
        return BlockFlag{.parent = this};
    }

    void EndBlock()
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);

        Block *m = &blocks[queue.Pop()];
        m->timeEx += now.QuadPart - m->from;
        m->timeInc += now.QuadPart - m->from;

        if (queue.len > 0)
        {
            Block *prev = &blocks[queue.Last()];
            prev->from = now.QuadPart;
            prev->timeInc += now.QuadPart - m->from;
        }
    }

    void End()
    {
        if (ended)
            return;

        ended = true;
        Initialized = false;

        LARGE_INTEGER perfCounter, perfFreq;
        QueryPerformanceCounter(&perfCounter);
        QueryPerformanceFrequency(&perfFreq);

        f64 totalTime =
            f64(perfCounter.QuadPart - start) / f64(perfFreq.QuadPart);

        INFO("Finished %s in %.6f seconds", name, totalTime);
        printf(" %-24s \t| %-25s \t| %-25s \t| %-12s\n",
               "Name[n]",
               "Time (Ex)",
               "Time (Inc)",
               "Bandwidth");
        printf(
            "-----------------------------------------------------------------------------------"
            "--------------------"
            "--------\n");

        for (u64 i = 1; i < blocks.cap; i++)
        {
            auto next = blocks[i];
            if (next.iterations == 0)
                continue;

            f64 nextTimeEx = (f64(next.timeEx) / f64(perfFreq.QuadPart));
            f64 nextTimeInc = (f64(next.timeInc) / f64(perfFreq.QuadPart));
            if (next.bytesProcessed == 0)
            {
                printf(" %-20s [%llu] \t| %.5f secs\t(%.2f%%) \t| %.5f secs\t(%.2f%%) \t|\n",
                       next.label,
                       next.iterations,
                       nextTimeEx,
                       (nextTimeEx / totalTime) * 100,
                       nextTimeInc,
                       (nextTimeInc / totalTime) * 100);
            }
            else
            {
                printf(
                    " %-20s [%llu] \t| %.5f secs\t(%.2f%%) \t| %.5f secs\t(%.2f%%) \t| %.3f GB/s\n",
                    next.label,
                    next.iterations,
                    nextTimeEx,
                    (nextTimeEx / totalTime) * 100,
                    nextTimeInc,
                    (nextTimeInc / totalTime) * 100,
                    f64(next.bytesProcessed) / nextTimeEx / 1024.0 / 1024.0 / 1024.0);
            }
        }
    }

    ~Profiler()
    {
        if (!Initialized)
        {
            Initialized = true;
            return;
        }
        if (start != 0)
            End();
    }
};

Profiler Profiler::_Profiler = {};
bool Profiler::Initialized = false;

typedef struct
{
    u64 time, bytes, pageFaults;
} RepBlock;

extern "C"
{
    i32 ByTime(const void *from, const void *to)
    {
        return ((RepBlock *)(from))->time - ((RepBlock *)(to))->time;
    }

    i32 ByBytes(const void *from, const void *to)
    {
        return ((RepBlock *)(from))->bytes - ((RepBlock *)(to))->bytes;
    }

    i32 ByPageFaults(const void *from, const void *to)
    {
        return ((RepBlock *)(from))->pageFaults - ((RepBlock *)(to))->pageFaults;
    }
}

static f64 ToGb(f64 bytes)
{
    return bytes / 1024.0 / 1024.0 / 1024.0;
}

struct RepProfiler
{
    cstr name;

    RepBlock first, min, max, avg, current;
    u64 repeats, maxRepeats;

    static RepProfiler New(cstr name, u64 maxRepeats = 100)
    {
        return RepProfiler{
            .name = name,
            .first = {},
            .min = {},
            .max = {},
            .avg = {},
            .current = {},
            .repeats = 0,
            .maxRepeats = maxRepeats,
        };
    }

    void BeginRep()
    {
        LARGE_INTEGER perfCounter;
        QueryPerformanceCounter(&perfCounter);
        current = RepBlock{
            .time = u64(perfCounter.QuadPart),
            .bytes = 0,
            .pageFaults = Metrics::Get().ReadPageFaultCount(),
        };
    }

    void AddBytes(u64 bytes) { current.bytes += bytes; }

    void EndRep()
    {
        LARGE_INTEGER perfCounter;
        QueryPerformanceCounter(&perfCounter);
        current.time = perfCounter.QuadPart - current.time;
        current.pageFaults = Metrics::Get().ReadPageFaultCount() - current.pageFaults;

        if (current.time < min.time || min.time == 0)
        {
            min = current;
        }

        if (current.time >= max.time)
        {
            max = current;
        }

        avg.bytes += current.bytes;
        avg.time += current.time;
        avg.pageFaults += current.pageFaults;

        if (repeats == 0)
            first = current;

        repeats++;
    }

    ~RepProfiler()
    {
        INFO("Finished %s after %llu repeats.", name, repeats);

        // FIRST
        LARGE_INTEGER perfFreq;
        QueryPerformanceFrequency(&perfFreq);

        f64 firstTime = f64(first.time) / f64(perfFreq.QuadPart);
        printf("\t> Initial: \t%.3f ms\t%.3f GB/s\t%llu pf\n",
               firstTime * 1000.0,
               ToGb(f64(first.bytes) / firstTime),
               first.pageFaults);

        // MIN
        f64 minTime = f64(min.time) / f64(perfFreq.QuadPart);
        printf("\t> Fastest: \t%.3f ms\t%.3f GB/s\t%llu pf\n",
               minTime * 1000.0,
               ToGb(f64(min.bytes) / minTime),
               min.pageFaults);

        // MAX
        f64 maxTime = f64(max.time) / f64(perfFreq.QuadPart);
        printf("\t> Slowest: \t%.3f ms\t%.3f GB/s\t%llu pf\n",
               maxTime * 1000.0,
               ToGb(f64(max.bytes) / maxTime),
               max.pageFaults);

        // AVERAGE
        f64 avgBytes = f64(avg.bytes) / f64(repeats);
        f64 avgFaults = f64(avg.pageFaults) / f64(repeats);
        f64 avgTime = f64(avg.time) / f64(repeats);
        avgTime /= f64(perfFreq.QuadPart);

        printf("\t> Average: \t%.3f ms\t%.3f GB/s\t%.2f pf\n",
               avgTime * 1000.0,
               ToGb(f64(avgBytes) / avgTime),
               avgFaults);
    }
};

#ifndef DISABLE_PROFILER

#define PROFILER_NEW(name) Profiler::New(name)
#define PROFILER_END() Profiler::Get().End()
#define PROFILE_BLOCK_BEGIN(name) \
    Profiler::Get().BeginBlock(__COUNTER__ + 1, name, __FILE__, __LINE__)
#define PROFILE_ADD_BANDWIDTH(bytes) Profiler::Get().AddBytes(bytes)
#define PROFILE_BLOCK_END() Profiler::Get().EndBlock()
#define PROFILE_SCOPE(name) \
    auto _profilerFlag = Profiler::Get().BeginScopeBlock(__COUNTER__ + 1, name, __FILE__, __LINE__)
#define PROFILE_FUNCTION() \
    auto _profilerFlag =   \
        Profiler::Get().BeginScopeBlock(__COUNTER__ + 1, __func__, __FILE__, __LINE__)
#define PROFILE(name, code)                                                \
    Profiler::Get().BeginBlock(__COUNTER__ + 1, name, __FILE__, __LINE__); \
    code;                                                                  \
    Profiler::Get().EndBlock();

#define REPETITION_PROFILE(name, count)                  \
    do                                                   \
    {                                                    \
        auto _profiler = RepProfiler::New(name, count);  \
        while (_profiler.repeats < _profiler.maxRepeats) \
        {                                                \
            _profiler.BeginRep();

#define REPETITION_BANDWIDTH(bytes) _profiler.AddBytes(bytes)

#define REPETITION_END() \
    _profiler.EndRep();  \
    }                    \
    }                    \
    while (0)            \
        ;

#else

#define PROFILER_NEW(...)
#define PROFILER_END(...)
#define PROFILE_BLOCK_BEGIN(...)
#define PROFILE_ADD_BANDWIDTH(...)
#define PROFILE_BLOCK_END(...)
#define PROFILE_SCOPE(...)
#define PROFILE_FUNCTION(...)
#define PROFILE(name, code) code

#define REPETITION_PROFILE(...)
#define REPETITION_BANDWIDTH(...)
#define REPETITION_END(...)

#endif
