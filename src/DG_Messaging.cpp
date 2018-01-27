#include "DG_Messaging.h"
namespace DG
{
void MessagingSystem::Update()
{
    for (auto& updateCallbacks : _updateCalls) updateCallbacks(_clock);
}
}  // namespace DG
