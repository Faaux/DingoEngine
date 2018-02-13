/**
 *  @file    ConditionVariable.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    13 February 2018
 */

#pragma once
#include <SDL.h>
namespace DG
{
class ConditionVariable
{
   public:
    void Create();
    void Destroy();

    void Signal();
    void WaitAndReset();

   private:
    SDL_mutex *_lock = nullptr;
    SDL_cond *_cond = nullptr;
    bool _condition = false;
};
}  // namespace DG
