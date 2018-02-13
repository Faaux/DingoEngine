/**
 *  @file    ConditionVariable.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    13 February 2018
 */

#include "ConditionVariable.h"
#include "engine/Types.h"

namespace DG
{
void ConditionVariable::Create()
{
    Assert(!_lock && !_cond);
    _lock = SDL_CreateMutex();
    _cond = SDL_CreateCond();
}

void ConditionVariable::Destroy()
{
    Assert(_lock && _cond);
    SDL_DestroyCond(_cond);
    SDL_DestroyMutex(_lock);
    _cond = nullptr;
    _lock = nullptr;
}

void ConditionVariable::Signal()
{
    Assert(_lock && _cond);
    SDL_LockMutex(_lock);
    _condition = true;
    SDL_CondSignal(_cond);
    SDL_UnlockMutex(_lock);
}

void ConditionVariable::WaitAndReset()
{
    Assert(_lock && _cond);
    SDL_LockMutex(_lock);
    while (!_condition)
    {
        SDL_CondWait(_cond, _lock);
    }
    _condition = false;
    SDL_UnlockMutex(_lock);
}
}  // namespace DG
