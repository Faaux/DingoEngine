/**
 *  @file    Messaging.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include <functional>
#include <optional>
#include <queue>
#include "engine/Types.h"
#include "platform/Clock.h"

namespace DG
{
template <class T>
class CallbackHandle
{
    friend class MessagingSystem;

   public:
    CallbackHandle() = default;
    ~CallbackHandle() = default;
    CallbackHandle(const CallbackHandle&) = delete;
    CallbackHandle(CallbackHandle&& other) noexcept : _index(other._index) { other._index.reset(); }
    CallbackHandle& operator=(const CallbackHandle&) = delete;
    CallbackHandle& operator=(CallbackHandle&& other) noexcept = delete;

    bool IsValid() const { return _index.has_value(); }

   private:
    CallbackHandle(u32 index) : _index(index) {}

    std::optional<u32> _index;
};

class MessagingSystem
{
   public:
    MessagingSystem() = default;
    void Init(const Clock& clock);
    void Update();

    template <class T>
    void Send(const T& message, float delayInS);

    template <class T>
    void SendNextFrame(const T& message);

    template <class T>
    void SendImmediate(const T& message);

    template <class T>
    CallbackHandle<T> RegisterCallback(std::function<void(const T&)> callback);
    template <class T>
    void UnRegisterCallback(CallbackHandle<T>& handle);

   private:
    template <class T>
    struct InternalMessage
    {
        InternalMessage(u64 delay, const T& message) : timeToSend(delay), message(message) {}
        u64 timeToSend;
        T message;
        friend inline bool operator<(const InternalMessage& lhs, const InternalMessage& rhs)
        {
            return lhs.timeToSend > rhs.timeToSend;
        }
        friend inline bool operator>(const InternalMessage& lhs, const InternalMessage& rhs)
        {
            return rhs < lhs;
        }
        friend inline bool operator<=(const InternalMessage& lhs, const InternalMessage& rhs)
        {
            return !(lhs > rhs);
        }
        friend inline bool operator>=(const InternalMessage& lhs, const InternalMessage& rhs)
        {
            return !(lhs < rhs);
        }
    };

    const Clock* _clock = nullptr;

    std::vector<std::function<void(const Clock*)>> _updateCalls;

    template <class T>
    void InternalSend(const T& message);

    template <class T>
    static std::priority_queue<InternalMessage<T>> _messageQueue;

    template <class T>
    static std::vector<std::function<void(const T&)>> _messageCallbacks;

    template <class T>
    static std::vector<u32> _messageCallbacksFreeList;

    template <class T>
    static void UpdateTypedMessageQueue(const Clock* clock);
};
template <class T>
std::vector<u32> MessagingSystem::_messageCallbacksFreeList;

template <class T>
std::priority_queue<MessagingSystem::InternalMessage<T>> MessagingSystem::_messageQueue;

template <class T>
std::vector<std::function<void(const T&)>> MessagingSystem::_messageCallbacks;

template <class T>
void MessagingSystem::Send(const T& message, float delayInS)
{
    if (delayInS < 0.0f)
    {
        InternalSend(message);
    }
    else
    {
        u64 cycles = _clock->GetTimeCycles() + _clock->ToCycles(delayInS);
        _messageQueue<T>.emplace(cycles, message);
    }
}

template <class T>
void MessagingSystem::SendNextFrame(const T& message)
{
    Send(message, 0.f);
}

template <class T>
void MessagingSystem::SendImmediate(const T& message)
{
    Send(message, -1.f);
}

template <class T>
CallbackHandle<T> MessagingSystem::RegisterCallback(std::function<void(const T&)> callback)
{
    static bool didRegister = false;
    if (!didRegister)
    {
        didRegister = true;
        _updateCalls.push_back(&MessagingSystem::UpdateTypedMessageQueue<T>);
    }

    auto& callbacks = _messageCallbacks<T>;
    auto& freeList = _messageCallbacksFreeList<T>;
    if (!freeList.empty())
    {
        u32 index = freeList.back();
        freeList.pop_back();

        callbacks[index] = callback;
        return CallbackHandle<T>(index);
    }
    else
    {
        callbacks.push_back(callback);
        return CallbackHandle<T>((u32)(callbacks.size()) - 1);
    }
}

template <class T>
void MessagingSystem::UnRegisterCallback(CallbackHandle<T>& handle)
{
    auto& index = handle._index.value();
    _messageCallbacks<T>[index] = nullptr;
    _messageCallbacksFreeList<T>.push_back(index);
    handle._index.reset();
    Assert(!handle.IsValid());
}

template <class T>
void MessagingSystem::InternalSend(const T& message)
{
    for (auto& callback : _messageCallbacks<T>)
    {
        if (callback)
            callback(message);
    }
}

template <class T>
void MessagingSystem::UpdateTypedMessageQueue(const Clock* clock)
{
    const u64 currentTimeInCylces = clock->GetTimeCycles();
    while (!_messageQueue<T>.empty())
    {
        const InternalMessage<T>& message = _messageQueue<T>.top();
        if (message.timeToSend > currentTimeInCylces)
            break;

        auto& callbacks = _messageCallbacks<T>;
        for (auto& callback : callbacks)
            if (callback)
                callback(message.message);

        _messageQueue<T>.pop();
    }
}

extern MessagingSystem g_MessagingSystem;
}  // namespace DG
