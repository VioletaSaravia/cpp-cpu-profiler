#pragma once

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

    Profiler(cstr _name = "");
    void BeginBlock(u64 id, cstr label = "", cstr file = "", i32 line = 0, u64 bytesProcessed = 0);
    void AddBytes(u64 bytes);
    BlockFlag
    BeginScopeBlock(i32 id, cstr label, cstr file = "", i32 line = 0, u64 bytesProcessed = 0);
    void EndBlock();
    void End();
    ~Profiler();
};

struct RepBlock
{
    u64 time, bytes, pageFaults;
};

struct RepProfiler
{
    cstr name;
    RepBlock first, min, max, avg, current;
    u64 repeats, maxRepeats;

    static RepProfiler New(cstr name, u64 maxRepeats = 100);
    void BeginRep();
    void AddBytes(u64 bytes);
    void EndRep();
    ~RepProfiler();
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
