#pragma once
#include <map>
#include <queue>
#include <stack>
#include "DG_Include.h"

namespace DG
{
//  Windows
#ifdef _WIN32
#include <intrin.h>
#define RDTSC __rdtsc

//  Linux/GCC
#else
#include <x86intrin.h>
#define RDTSC _rdtsc
#endif

enum DebugEventType
{
    FrameStart,
    FrameEnd,
    Start,
    End,
    Error
};

int ProfilerWork(void* data, FILE* output_stream);

struct CombinedEvaluatedDebugEvent
{
    u64 TotalElapsedCycles = 0;
    u64 MaxElapsedCycles = 0;
    std::string_view Name;
    std::string_view FileName;
    u32 LineNumber;
    u32 Counter;
};

struct EvaluatedDebugEvent
{
    u64 StartClock = 0;
    u64 ElapsedCycles = 0;

    u32 Level = 0;

    std::string_view Name;
    std::string_view FileName;
    u32 LineNumber;
    u32 Counter;
};

struct DebugEvent
{
    DebugEvent(u64 clock, u32 threadId, std::string_view guid, DebugEventType type,
               std::string_view name = "", std::string_view file = "", u32 line = 0, u32 counter = 0)
        : Clock(clock),
          ThreadId(threadId),
          GUID(guid),
          Type(type),
          Name(name),
          FileName(file),
          LineNumber(line),
          Counter(counter)
    {
    }

    u64 Clock;
    u32 ThreadId;
    std::string_view GUID;
    DebugEventType Type;
    std::string_view Name;
    std::string_view FileName;
    u32 LineNumber;
    u32 Counter;
};

struct DebugFrame
{
    u64 FrameId = 0;
    u64 StartCycle = 0;
    u64 TotalCycles = 0;
    std::map<u32, std::map<std::string_view, CombinedEvaluatedDebugEvent>> AveragedEvents;
    std::map<u32, std::vector<EvaluatedDebugEvent>> Events;
};

class DebugProfiler
{
    friend int ProfilerWork(void* data, FILE* output_stream);
    friend void PushEvent(DebugEventType type, std::string_view guid, std::string_view name, std::string_view file,
                          u32 line, u32 counter);

   public:
    enum
    {
        FramesRecorded = 300
    };

    DebugProfiler()
    {
        _semaphore = SDL_CreateSemaphore(0);
        _mutex = SDL_CreateMutex();
        _processor_frequency = (double)SDL_GetPerformanceFrequency();
    }

    ~DebugProfiler()
    {
        SDL_DestroySemaphore(_semaphore);
        SDL_DestroyMutex(_mutex);
    }

    const DebugFrame* GetCurrentFrame() const
    {
        if (_lastCompleteFrame == -1) return nullptr;

        return &_frames[_lastCompleteFrame];
    }

    const DebugFrame* GetFrame(s32 frame) const
    {
        Assert(frame >= 0 && frame < FramesRecorded);
        return &_frames[frame];
    }

    void ToggleRecording()
    {
        _hasFrameStart = false;
        _stopRecording = !_stopRecording;
    }

    void TurnOffRecording()
    {
        _hasFrameStart = false;
        _stopRecording = true;
    }

    DebugEvent GetNextEvent();

    s32 FrameToDebug = 0;

   private:
    DebugProfiler(const DebugProfiler&) = delete;
    void operator=(const DebugProfiler&) = delete;

    std::map<u32, std::stack<DebugEvent>> _builderStacks;
    std::map<u32, std::map<std::string_view, CombinedEvaluatedDebugEvent>> _concatenatedResultMap;
    std::map<u32, std::vector<EvaluatedDebugEvent>> _resultMap;
    std::queue<DebugEvent> _queue;
    SDL_sem* _semaphore;
    SDL_mutex* _mutex;

    DebugFrame _frames[FramesRecorded];
    u32 _currentIndex = 0;
    s32 _lastCompleteFrame = -1;
    double _processor_frequency;
    bool _stopRecording = false;
    bool _hasFrameStart = false;
};

DebugProfiler* GetProfiler();

inline void PushEvent(DebugEventType type, std::string_view guid, std::string_view name = "",
                      std::string_view file = "", u32 line = 0, u32 counter = 0)
{
    DebugProfiler* profiler = GetProfiler();
    Assert(profiler->_semaphore && profiler->_mutex);
    u32 threadID = SDL_ThreadID();

    SDL_LockMutex(profiler->_mutex);
    profiler->_queue.emplace(SDL_GetPerformanceCounter(), threadID, guid, type, name, file, line,
                             counter);
    SDL_UnlockMutex(profiler->_mutex);

    SDL_SemPost(profiler->_semaphore);
}

#define UniqueFileCounterString__(A, B, C, D) A "|" #B "|" #C "|" D
#define UniqueFileCounterString_(A, B, C, D) UniqueFileCounterString__(A, B, C, D)
#define DEBUG_NAME(Name) UniqueFileCounterString_(__FILE__, __LINE__, __COUNTER__, Name)

#define TIMED_BLOCK__(GUID, Number, Name, ...) \
    timed_block TimedBlock_##Number(GUID, Name, __FILE__, __LINE__, Number, ##__VA_ARGS__)
#define TIMED_BLOCK_(GUID, Number, Name, ...) TIMED_BLOCK__(GUID, Number, Name, ##__VA_ARGS__)
#define TIMED_BLOCK(Name, ...) TIMED_BLOCK_(DEBUG_NAME(Name), __COUNTER__, Name, ##__VA_ARGS__)
#define TIMED_FUNCTION(...) \
    TIMED_BLOCK_(DEBUG_NAME(__FUNCTION__), __COUNTER__, __FUNCTION__, ##__VA_ARGS__)

#define BEGIN_BLOCK_(GUID, NAME, FILE, LINENUMBER, COUNTER)      \
    {                                                            \
        PushEvent(Start, GUID, NAME, FILE, LINENUMBER, COUNTER); \
    }
#define END_BLOCK_(GUID)      \
    {                         \
        PushEvent(End, GUID); \
    }

#define BEGIN_BLOCK(Name) BEGIN_BLOCK_(DEBUG_NAME(Name), Name, __FILE__, __LINE__, __COUNTER__)
#define END_BLOCK() END_BLOCK_(DEBUG_NAME("END_BLOCK_"))

#define FRAME_START()                                                                       \
    {                                                                                       \
        PushEvent(FrameStart, DEBUG_NAME("Frame Start"), "Frame Start", __FILE__, __LINE__, \
                  __COUNTER__);                                                             \
    }
#define FRAME_END()                                                \
    {                                                              \
        PushEvent(FrameEnd, DEBUG_NAME("Frame End"), "Frame End"); \
    }

// p:\dingo\dgludus\src\ecs/DG_World.h|189|1|DG::World::AddComponent

struct timed_block
{
    timed_block(char* GUID, char* name, char* file, u32 lineNumber, u32 counter)
    {
        BEGIN_BLOCK_(GUID, name, file, lineNumber, counter);
    }

    ~timed_block() { END_BLOCK(); }
};
}  // namespace DG
