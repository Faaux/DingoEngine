/**
*  @file    DG_Memory.h
*  @author  Faaux (github.com/Faaux)
*  @date    11 February 2018
*/

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
        _current = base + size;
        _size = size;
    }

    u8* Push(u32 size, u32 alignment);
    void Pop(void* ptr);

    void Reset();

    template <typename T, typename... Args>
    T* PushAndConstruct(Args&&... args);

    template <typename T>
    T* Push(u32 count = 1);

   private:
    struct StackHeader
    {
        bool free;
        u32 size;
    };

    bool _isInitialized = false;
    u8* _base = 0;
    u8* _current = 0;
    u32 _size = 0;
    u32 _hwm = 0;
};

template <typename T, typename... Args>
T* StackAllocator::PushAndConstruct(Args&&... args)
{
    void* memory = Push(sizeof(T), 4);
    return new (memory) T(std::forward<Args>(args)...);
}

template <typename T>
T* StackAllocator::Push(u32 count)
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
