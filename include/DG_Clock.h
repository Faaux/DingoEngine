#pragma once
#include "DG_Include.h"

namespace DG
{
class Clock
{
   public:
    static void Init();
    explicit Clock(r32 startTime = 0.0f);

    u64 GetTimeCycles() const;
    r32 CalcDeltaSeconds(const Clock& other) const;

    void Update(r32 dtRealSeconds);

    void SetPaused(bool wantPaused);
    bool IsPaused() const;

    void SetTimeScale(r32 wantedTimeScale);
    r32 GetTimeScale() const;

    void SingleStep();

   private:
    u64 _timeCycles;
    r32 _timeScale;
    bool _isPaused;

    static r32 _cyclesPerSecond;
    static inline u64 SecondsToCycles(r32 seconds);
    static inline r32 CyclesToSeconds(u64 cycles);
};

extern Clock g_RealTimeClock;
extern Clock g_InGameClock;
extern Clock g_AnimationClock;
void InitClocks();
}  // namespace DG
