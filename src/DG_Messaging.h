#pragma once
#include <functional>
#include <queue>
#include "DG_Clock.h"
#include "DG_Include.h"

namespace DG
{
struct StringMessage
{
    std::string message;
};

class MessagingSystem
{
   public:
    MessagingSystem(const Clock& clock) : _clock(clock){};
    void Update();

    template <class T>
    void Send(const T& message, float delayInS = 0.0f);

    template <class T>
    void RegisterCallback(std::function<void(const T&)> callback);

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

    const Clock& _clock;

    std::vector<std::function<void(const Clock&)>> _updateCalls;

    template <class T>
    void InternalSend(const T& message);

    template <class T>
    static std::priority_queue<InternalMessage<T>> _messageQueue;

    template <class T>
    static std::vector<std::function<void(const T&)>> _messageCallbacks;

    template <class T>
    static void UpdateTypedMessageQueue(const Clock& clock);
};
template <class T>
std::priority_queue<MessagingSystem::InternalMessage<T>> MessagingSystem::_messageQueue;

template <class T>
std::vector<std::function<void(const T&)>> MessagingSystem::_messageCallbacks;

template <class T>
void MessagingSystem::Send(const T& message, float delayInS)
{
    static bool didRegister = false;
    if (!didRegister)
    {
        didRegister = true;
        _updateCalls.push_back(&MessagingSystem::UpdateTypedMessageQueue<T>);
    }

    if (delayInS == 0.0f)
    {
        InternalSend(message);
    }
    else
    {
        u64 cycles = _clock.GetTimeCycles() + _clock.ToCycles(delayInS);
        _messageQueue<T>.emplace(cycles, message);
    }
}

template <class T>
void MessagingSystem::RegisterCallback(std::function<void(const T&)> callback)
{
    _messageCallbacks<T>.push_back(callback);
}

template <class T>
void MessagingSystem::InternalSend(const T& message)
{
    for (auto& callback : _messageCallbacks<T>)
    {
        callback(message);
    }
}

template <class T>
void MessagingSystem::UpdateTypedMessageQueue(const Clock& clock)
{
    const u64 currentTimeInCylces = clock.GetTimeCycles();
    while (!_messageQueue<T>.empty())
    {
        const InternalMessage<T>& message = _messageQueue<T>.top();
        if (message.timeToSend > currentTimeInCylces)
            break;

        for (auto& callback : _messageCallbacks<T>) callback(message.message);

        _messageQueue<T>.pop();
    }
}
}  // namespace DG
