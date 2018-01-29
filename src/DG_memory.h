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

    template <typename T, typename... Args>
    T* PushAndConstruct(Args&&... args);

    template <typename T>
    T* Push();

   private:
    bool _isInitialized = false;
    u8* _base = 0;
    u32 _currentUse = 0;
    u32 _size = 0;
};

template <typename T, typename... Args>
T* StackAllocator::PushAndConstruct(Args&&... args)
{
    Assert(_isInitialized);
    Assert(_currentUse + sizeof(T) <= _size);

    T* result = new (_base + _currentUse) T(std::forward(args)...);
    _currentUse += sizeof(T);

    return result;
}

template <typename T>
T* StackAllocator::Push()
{
    Assert(_isInitialized);
    Assert(_currentUse + sizeof(T) <= _size);
    SDL_memset(_base + _currentUse, 0, sizeof(T));
    T* result = reinterpret_cast<T*>(_base + _currentUse);
    _currentUse += sizeof(T);
    return result;
}

struct GameMemory
{
    StackAllocator PersistentMemory;
    StackAllocator TransientMemory;
};
}  // namespace DG
