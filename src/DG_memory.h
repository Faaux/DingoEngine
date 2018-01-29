#pragma once
#include "DG_Include.h"
namespace DG
{
struct StackAllocator
{
   public:
    void Init(u8* base, u32 size)
    {
        Assert(!_isInitialized);
        _isInitialized = true;
        _base = base;
        _size = size;
        _currentUse = 0;
    }

    void* Push(u32 size, u32 alignment);
    void Pop(void* ptr);

    template <typename T, typename... Args>
    T* PushAndConstruct(Args&&... args);

    template <typename T, u32 count = 1>
    T* Push();

   private:
    struct StackHeader
    {
        u8 padding;
    };

    bool _isInitialized = false;
    u8* _base = 0;
    u32 _currentUse = 0;
    u32 _size = 0;
};

template <typename T, typename... Args>
T* StackAllocator::PushAndConstruct(Args&&... args)
{
    void* memory = Push(sizeof(T), 4);
    return new (memory) T(std::forward(args)...);
}

template <typename T, u32 count>
T* StackAllocator::Push()
{
    void* memory = Push(sizeof(T) * count, 4);
    SDL_memset(memory, 0, sizeof(T) * count);
    return reinterpret_cast<T*>(memory);
}

struct GameMemory
{
    StackAllocator PersistentMemory;
    StackAllocator TransientMemory;
};
}  // namespace DG
