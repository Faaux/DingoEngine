/**
 *  @file    Memory.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "Memory.h"
#include "math/GLMInclude.h"
#include "platform/BitOperations.h"

namespace DG
{
u8* StackAllocator::Push(u32 size, u32 alignment)
{
    if (size == 0)
        return nullptr;

    Assert(_isInitialized);

    size_t baseAddress = (size_t)(_current - size);
    size_t padding = (((baseAddress / alignment) + 1) * alignment) - baseAddress;
    padding = alignment - padding;
    Assert(padding >= 0);
    Assert(padding <= 255);

    Assert(_current + (size + padding + sizeof(StackHeader)) >= _base);

    u8* data = _current - (size + padding);
    StackHeader* header = (StackHeader*)(data - sizeof(StackHeader));
    header->free = false;
    header->size = size + (u32)padding;

    _current = (u8*)header;

    Assert((size_t)data % alignment == 0);
    _hwm = glm::max((u32)((size_t)(_base + _size) - (size_t)_current), _hwm);

    SDL_memset(data, 0, size);
    return data;
}

void StackAllocator::Pop(void* p)
{
    Assert(_isInitialized);
    u8* data = (u8*)p;
    StackHeader* header = (StackHeader*)(data - sizeof(StackHeader));
    header->free = true;

#if _DEBUG
    SDL_memset4(data, 0xDEADBEEF, header->size / 4);
#endif

    // Pop everything off that was freed
    header = (StackHeader*)_current;
    while ((u8*)header != _base + _size && header->free)
    {
        _current = _current + (header->size + sizeof(StackHeader));
        header = (StackHeader*)_current;
    }

    Assert(_base + _size >= _current);
}

void StackAllocator::Reset() { _current = _base + _size; }

void BasePoolAllocator::Initialize(StackAllocator* allocator, s32 size)
{
    Assert(!_isInitialized);
    Assert(_size % 4 == 0);
    _isInitialized = true;
    _allocator = allocator;
    _size = size;

    _head = AllocateNewChunk();
}

void BasePoolAllocator::Shutdown()
{
    Chunk* current = _head;
    Chunk* next = _head->Next;
    _allocator->Pop(current->Base);
    _allocator->Pop(current);
    while (next)
    {
        current = next;
        next = next->Next;
        _allocator->Pop(current->Base);
        _allocator->Pop(current);
    }
}

u8* BasePoolAllocator::Allocate()
{
    Assert(_isInitialized);

    // Find a chunk with space
    Chunk* candidate = _head;
    while (candidate->Bitmask == 0)
    {
        if (!candidate->Next)
            candidate->Next = AllocateNewChunk();
        candidate = candidate->Next;
    }

    u32 index = 0;
    BitScanForward(candidate->Bitmask, &index);

    u8* data = candidate->Base + (_size * index);
    u64 bit = (u64)1 << index;
    candidate->Bitmask &= ~bit;

    return data;
}

void BasePoolAllocator::Free(void* ptr)
{
    Assert(_isInitialized);

    // Find correct chunk
    Chunk* candidate = _head;
    while (candidate && (candidate->Base > ptr || candidate->Base + 63 * _size < ptr))
    {
        candidate = candidate->Next;
    }
    Assert(candidate);

    u64 index = ((u64)ptr - (u64)candidate->Base) / _size;
    u64 bit = (u64)1 << index;
    candidate->Bitmask |= bit;

#if _DEBUG
    SDL_memset4(ptr, 0xDEADBEEF, _size / 4);
#endif
}

BasePoolAllocator::Iterator<u8> BasePoolAllocator::begin() const
{
    return Iterator<u8>(_head, _size);
}
BasePoolAllocator::Iterator<u8> BasePoolAllocator::end() const
{
    return Iterator<u8>(nullptr, _size);
}

BasePoolAllocator::Chunk* BasePoolAllocator::AllocateNewChunk() const
{
    Assert(_isInitialized);
    Chunk* chunk = _allocator->Push<Chunk>();

    chunk->Next = nullptr;
    chunk->Bitmask = 0xFFFFFFFFFFFFFFFF;
    chunk->Base = _allocator->Push(64 * _size, 16);

    return chunk;
}
}  // namespace DG
