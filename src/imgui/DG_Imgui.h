#pragma once
#include <vector>
#include "DG_Include.h"
namespace DG
{
enum TweakerType: u8
{
    CB,
    F1,
    F2,
    F3,
    F4,
    S1,
    S2,
    S3,
    S4,
    Color3Big,
    Color3Small,
    Color4Big,
    Color4Small,
};
struct Tweaker
{
    Tweaker(TweakerType type, const char *name, void *ptr) : Type(type), Name(name), Ptr(ptr){};

    TweakerType Type;
    const char *Name;
    void *Ptr;
};

#define RUN_ONCE(runcode)             \
    do                                \
    {                                 \
        static bool code_ran = false; \
        if (!code_ran)                \
        {                             \
            code_ran = true;          \
            runcode;                  \
        }                             \
    } while (0)

void InitInternalImgui();
void AddImguiTweakers();
extern std::vector<Tweaker> g_Tweakers;
extern std::vector<Tweaker> g_TweakersPerFrame;
#define TWEAKER(Type, Name, Ptr) RUN_ONCE(g_Tweakers.emplace_back(Type, Name, Ptr);)
#define TWEAKER_FRAME(Type, Name, Ptr) g_TweakersPerFrame.emplace_back(Type, Name, Ptr)
}  // namespace DG
