/**
 *  @file    Messaging.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once

#include <queue>
#include <unordered_map>
#include "engine/Types.h"
#include "memory/Memory.h"
#include "platform/Clock.h"
#include "platform/Delegate.h"

namespace DG
{
class Key;

enum class MessageType : u8
{
    Undefined = 0,
    Fullscreen,
    RawKey,
    RawWindowSize,
    RawInput,
};

struct Message
{
    union {
        struct
        {
            char Padding[55];
        } Base;

        struct
        {
            Key* Key;
            SDL_Scancode Scancode;
            bool WasCtrlDown;
            bool WasAltDown;
        } RawKey;

        struct
        {
            s32 Width;
            s32 Height;
        } RawWindowSize;

        struct
        {
            bool SetFullScreen;
        } Fullscreen;

        struct
        {
            float Right;
            float Forward;
            float Up;
            float MouseWheel;
            bool MouseLeftDown;
            bool MouseRightDown;
            bool MouseMiddleDown;
            bool MouseLeftPressed;
            bool MouseRightPressed;
            bool MouseMiddlePressed;
            s32 MouseDeltaX;
            s32 MouseDeltaY;
            s32 MouseX;
            s32 MouseY;
        } RawInput;

        struct
        {
            float Right;
            float Forward;
            float Up;
            float MouseWheel;
            bool MouseLeftDown;
            bool MouseRightDown;
            bool MouseMiddleDown;
            bool MouseLeftPressed;
            bool MouseRightPressed;
            bool MouseMiddlePressed;
            s32 MouseDeltaX;
            s32 MouseDeltaY;
            s32 MouseX;
            s32 MouseY;
        } Input;
    };
    MessageType Type = MessageType::Undefined;
};

static_assert(sizeof(Message) == 64, "Messages must be 64 bytes big");

struct MessageHandle
{
    MessageType Type;
    s32 ID;
    void* Index;
};

class MessagingSystem
{
   public:
    void Initialize(StackAllocator* allocator, const Clock* clock);
    void Shutdown();

    void Update();

    void Send(const Message& message, f32 delay);
    void SendImmediate(Message message);
    void SendNextFrame(Message message);

    MessageHandle RegisterCallback(MessageType type,
                                   const Delegate<void(const Message&)>& callback);
    void UnregisterCallback(MessageHandle handle);

   private:
    void SendInternal(const Message& message);

    struct InternalMessage
    {
        InternalMessage(Message* message, u64 timeToSend) : Message(message), TimeToSend(timeToSend)
        {
        }
        Message* Message;
        u64 TimeToSend;

        friend bool operator<(const InternalMessage& lhs, const InternalMessage& rhs)
        {
            return lhs.TimeToSend > rhs.TimeToSend;
        }

        friend bool operator<=(const InternalMessage& lhs, const InternalMessage& rhs)
        {
            return !(rhs < lhs);
        }

        friend bool operator>(const InternalMessage& lhs, const InternalMessage& rhs)
        {
            return rhs < lhs;
        }

        friend bool operator>=(const InternalMessage& lhs, const InternalMessage& rhs)
        {
            return !(lhs < rhs);
        }
    };

    struct InternalDelgate
    {
        s32 ID;
        Delegate<void(const Message&)> Callback;
    };

    PoolAllocator<Message> _messagePool;
    std::priority_queue<InternalMessage> _messageQueue;
    std::unordered_map<MessageType, PoolAllocator<InternalDelgate>> _callbackMap;
    bool _isInitialized = false;
    const Clock* _clock = nullptr;
    StackAllocator* _allocator = nullptr;

    // ToDo: Make atomic
    inline static s32 CurrentDelegateID = 1;
};

extern MessagingSystem g_MessagingSystem;
}  // namespace DG
