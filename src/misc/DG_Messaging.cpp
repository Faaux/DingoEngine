#include "DG_Messaging.h"
namespace DG
{
MessagingSystem g_MessagingSystem;
void MessagingSystem::Init(const Clock& clock) { _clock = &clock; }

void MessagingSystem::Update()
{
    for (auto& updateCallbacks : _updateCalls) updateCallbacks(_clock);
}
}  // namespace DG
