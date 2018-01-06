#pragma once
#include "DG_Include.h"

namespace DG
{
class Clock
{
   public:
    static void Init();
    explicit Clock(f32 startTime = 0.0f);

    u64 GetTimeCycles() const;
    f32 CalcDeltaSeconds(const Clock& other) const;

    void Update(f32 dtRealSeconds);

    void SetPaused(bool wantPaused);
    bool IsPaused() const;

    void SetTimeScale(f32 wantedTimeScale);
    f32 GetTimeScale() const;

    void SingleStep();

   private:
    u64 _timeCycles;
    f32 _timeScale;
    bool _isPaused;

    static f32 _cyclesPerSecond;
    static inline u64 SecondsToCycles(f32 seconds);
    static inline f32 CyclesToSeconds(u64 cycles);
};

extern Clock g_RealTimeClock;
extern Clock g_InGameClock;
extern Clock g_AnimationClock;
void InitClocks();
}  // namespace DG
