/**
 *  @file    Messaging.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "Messaging.h"
namespace DG
{
MessagingSystem g_MessagingSystem;
void MessagingSystem::Init(const Clock& clock) { _clock = &clock; }

void MessagingSystem::Update()
{
    for (auto& updateCallbacks : _updateCalls) updateCallbacks(_clock);
}
}  // namespace DG
