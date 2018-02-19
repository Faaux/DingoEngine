/**
 *  @file    Messaging.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "Messaging.h"
namespace DG
{
MessagingSystem g_MessagingSystem;

void MessagingSystem::Initialize(StackAllocator* allocator, const Clock* clock)
{
    _allocator = allocator;
    _messagePool.Initialize(allocator);
    _clock = clock;
    _isInitialized = true;
}

void MessagingSystem::Shutdown()
{
    Assert(_isInitialized);
    _messagePool.Shutdown();

    for (auto& pair : _callbackMap)
    {
        pair.second.Shutdown();
    }
}

void MessagingSystem::Update()
{
    Assert(_isInitialized);
    const u64 currentTimeInCylces = _clock->GetTimeCycles();
    while (!_messageQueue.empty())
    {
        const auto& message = _messageQueue.top();
        if (message.TimeToSend > currentTimeInCylces)
            break;

        SendInternal(*message.Message);

        _messageQueue.pop();
    }
}

void MessagingSystem::Send(const Message& message, f32 delay)
{
    Assert(_isInitialized);
    Assert(message.Type != MessageType::Undefined);
    Message* copy = _messagePool.Allocate();
    *copy = message;

    _messageQueue.emplace(copy, _clock->GetTimeCycles() + _clock->ToCycles(delay));
}

void MessagingSystem::SendImmediate(Message message)
{
    Assert(_isInitialized);
    Assert(message.Type != MessageType::Undefined);
    SendInternal(message);
}

void MessagingSystem::SendNextFrame(Message message)
{
    Assert(_isInitialized);
    Assert(message.Type != MessageType::Undefined);
    Send(message, 0.f);
}

MessageHandle MessagingSystem::RegisterCallback(MessageType type,
                                                const Delegate<void(const Message&)>& callback)
{
    Assert(_isInitialized);
    if (_callbackMap.find(type) == _callbackMap.end())
    {
        _callbackMap[type].Initialize(_allocator);
    }

    auto& pool = _callbackMap[type];
    InternalDelgate* del = pool.Allocate();
    del->ID = ++CurrentDelegateID;
    del->Callback = callback;

    const MessageHandle result{type, del->ID, del};
    return result;
}

void MessagingSystem::UnregisterCallback(MessageHandle handle)
{
    Assert(_isInitialized);
    Assert(handle.ID == ((InternalDelgate*)handle.Index)->ID);  // Valid handle
    Assert(_callbackMap.find(handle.Type) != _callbackMap.end());
    auto& pool = _callbackMap[handle.Type];
    pool.Free(handle.Index);
}

void MessagingSystem::SendInternal(const Message& message)
{
    Assert(_isInitialized);
    if (_callbackMap.find(message.Type) == _callbackMap.end())
    {
        // No one registered to get the callback!
        return;
    }
    auto& pool = _callbackMap[message.Type];

    for (auto& it : pool)
    {
        it.Callback(message);
    }
}
}  // namespace DG
