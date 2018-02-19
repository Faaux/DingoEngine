/**
 *  @file    Memory.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include "engine/Types.h"
#include "platform/BitOperations.h"

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
    return (T*)memory;
}

struct GameMemory
{
    StackAllocator PersistentMemory;
    StackAllocator TransientMemory;
};

class BasePoolAllocator
{
   private:
    struct Chunk
    {
        Chunk* Next;
        u64 Bitmask;
        u8* Base;
    };

   public:
    void Initialize(StackAllocator* allocator, s32 size);
    void Shutdown();

    u8* Allocate();
    void Free(void* ptr);

    template <typename T>
    class Iterator
    {
        friend class BasePoolAllocator;

       public:
        Iterator operator++()
        {
            MoveToNext();
            return *this;
        }
        Iterator operator++(int junk)
        {
            MoveToNext();
            return *this;
        }

        T& operator*() const;
        T* operator->() const;

        bool operator==(const Iterator& rhs) const
        {
            return _chunk == rhs._chunk && _negatedBitmask == rhs._negatedBitmask;
        }
        bool operator!=(const Iterator& rhs) const { return !(*this == rhs); }

        template <class R>
        explicit Iterator(const Iterator<R>& other)
            : _negatedBitmask(other._negatedBitmask),
              _chunk(other._chunk),
              _size(other._size),
              _ptr((T*)other._ptr)
        {
        }
        template <class R>
        Iterator& operator=(const Iterator<R>& other)
        {
            if (this == &other)
                return *this;
            _negatedBitmask = other._negatedBitmask;
            _chunk = other._chunk;
            _size = other._size;
            _ptr = (T*)other._ptr;
            return *this;
        }

       private:
        Iterator(Chunk* chunk, s32 size);
        void MoveToNext();

        u64 _negatedBitmask;
        Chunk* _chunk;
        s32 _size;
        T* _ptr;
    };

    Iterator<u8> begin() const;
    Iterator<u8> end() const;

   private:
    Chunk* AllocateNewChunk() const;

    bool _isInitialized = false;
    StackAllocator* _allocator = nullptr;
    Chunk* _head = nullptr;
    s32 _size = 0;
};

template <class T>
struct PoolAllocator : private BasePoolAllocator
{
    void Initialize(StackAllocator* allocator);
    void Shutdown();

    Iterator<T> begin() const;
    Iterator<T> end() const;

    T* Allocate();
    void Free(void* ptr);
};

template <typename T>
T& BasePoolAllocator::Iterator<T>::operator*() const
{
    Assert(_ptr);
    return *_ptr;
}

template <typename T>
T* BasePoolAllocator::Iterator<T>::operator->() const
{
    Assert(_ptr);
    return _ptr;
}

template <typename T>
BasePoolAllocator::Iterator<T>::Iterator(Chunk* chunk, s32 size)
    : _negatedBitmask(chunk ? ~chunk->Bitmask : 0), _chunk(chunk), _size(size), _ptr(nullptr)
{
    MoveToNext();
}

template <typename T>
void BasePoolAllocator::Iterator<T>::MoveToNext()
{
    while (_chunk)
    {
        if (_negatedBitmask == 0)
        {
            _chunk = _chunk->Next;
            _negatedBitmask = _chunk ? ~_chunk->Bitmask : 0;
            continue;
        }

        u32 index;
        if (BitScanForward(_negatedBitmask, &index))
        {
            // Slot is taken, return the pointer!
            const u64 bit = (u64)1 << index;
            _negatedBitmask &= ~bit;
            _ptr = (T*)(_chunk->Base + _size * index);
            return;
        }
    }
}

template <class T>
void PoolAllocator<T>::Initialize(StackAllocator* allocator)
{
    static_assert(sizeof(T) % 4 == 0, "Pool Allocator Types should always be multiple of 4");
    BasePoolAllocator::Initialize(allocator, sizeof(T));
}

template <class T>
void PoolAllocator<T>::Shutdown()
{
    BasePoolAllocator::Shutdown();
}

template <class T>
BasePoolAllocator::Iterator<T> PoolAllocator<T>::begin() const
{
    return (Iterator<T>)BasePoolAllocator::begin();
}

template <class T>
BasePoolAllocator::Iterator<T> PoolAllocator<T>::end() const
{
    return (Iterator<T>)BasePoolAllocator::end();
}

template <class T>
T* PoolAllocator<T>::Allocate()
{
    return (T*)BasePoolAllocator::Allocate();
}

template <class T>
void PoolAllocator<T>::Free(void* ptr)
{
    BasePoolAllocator::Free(ptr);
}
}  // namespace DG
