#include "DG_Clock.h"
namespace DG
{
Clock g_RealTimeClock;
Clock g_InGameClock;
Clock g_AnimationClock;

f32 Clock::_cyclesPerSecond;

void Clock::Init() { _cyclesPerSecond = (f32)SDL_GetPerformanceFrequency(); }

Clock::Clock(f32 startTime)
    : _timeCycles(SecondsToCycles(startTime)), _timeScale(1.0f), _isPaused(false)
{
}

u64 Clock::GetTimeCycles() const { return _timeCycles; }

f32 Clock::CalcDeltaSeconds(const Clock& other) const
{
    u64 dt = _timeCycles - other._timeCycles;
    return CyclesToSeconds(dt);
}

void Clock::Update(f32 dtRealSeconds)
{
    if (!_isPaused)
    {
        u64 dtScaled = SecondsToCycles(dtRealSeconds * _timeScale);
        _lastDtSeconds = dtRealSeconds;
        _timeCycles += dtScaled;
    }
}

f32 Clock::GetLastDtSeconds() const
{
    if (_isPaused)
        return 0.f;
    return _lastDtSeconds;
}

void Clock::SetPaused(bool wantPaused) { _isPaused = wantPaused; }

bool Clock::IsPaused() const { return _isPaused; }

void Clock::SetTimeScale(f32 wantedTimeScale) { _timeScale = wantedTimeScale; }

f32 Clock::GetTimeScale() const { return _timeScale; }

void Clock::SingleStep()
{
    if (_isPaused)
    {
        u64 dtCycles = SecondsToCycles(1.0f / TargetFrameRate * _timeScale);
        _timeCycles += dtCycles;
    }
}

u64 Clock::ToCycles(f32 seconds) const { return SecondsToCycles(seconds); }

u64 inline Clock::SecondsToCycles(f32 seconds) { return (u64)(seconds * _cyclesPerSecond); }

f32 Clock::CyclesToSeconds(u64 cycles) { return (f32)(cycles / _cyclesPerSecond); }

void InitClocks()
{
    Clock::Init();
    g_RealTimeClock.Init();
    g_InGameClock.Init();
    g_AnimationClock.Init();
}
}  // namespace DG
