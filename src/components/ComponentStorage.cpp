/**
 *  @file    ComponentStorage.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#include "ComponentStorage.h"

namespace DG
{
BaseComponentStorage::BaseComponentStorage(StackAllocator *allocator, u32 itemSize)
    : _itemSize(itemSize), _allocator(allocator)
{
}

void *BaseComponentStorage::CreateComponent()
{
    if (!_nextFree)
        AllocateNewChunk();

    void *component = _nextFree;
    _nextFree = *reinterpret_cast<BaseComponent **>(_nextFree);

    SDL_memset(component, 0, _itemSize);
    return component;
}

void BaseComponentStorage::ReleaseComponent(BaseComponent *component)
{
    *reinterpret_cast<void **>(component) = _nextFree;
    _nextFree = component;
}

void BaseComponentStorage::AllocateNewChunk()
{
    ChunkHeader **c = &_baseChunk;
    while (*c) c = &((*c)->Next);

    u8 *chunk = _allocator->Push(sizeof(ChunkHeader) + _itemSize * ChunkSize, 4);
    ChunkHeader *allocatedChunk = reinterpret_cast<ChunkHeader *>(chunk);
    *c = allocatedChunk;
    allocatedChunk->Data = reinterpret_cast<u8 *>(chunk + sizeof(ChunkHeader));

    _nextFree = (BaseComponent *)allocatedChunk->Data;
    for (u32 i = 0; i < ChunkSize - 1; ++i)
    {
        *reinterpret_cast<void **>(allocatedChunk->Data + i * _itemSize) =
            (allocatedChunk->Data + i * _itemSize + 1);
    }

    *reinterpret_cast<void **>(allocatedChunk->Data + (ChunkSize - 1) * _itemSize) = nullptr;
}
}  // namespace DG
