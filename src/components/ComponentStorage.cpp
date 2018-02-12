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
    _nextFree = *(BaseComponent **)_nextFree;

    SDL_memset(component, 0, _itemSize);
    return component;
}

void BaseComponentStorage::ReleaseComponent(BaseComponent *component)
{
    *(BaseComponent **)component = _nextFree;
    _nextFree = component;
}

void BaseComponentStorage::AllocateNewChunk()
{
    ChunkHeader **c = &_baseChunk;
    while (*c) c = &((*c)->Next);

    u8 *chunk = _allocator->Push(sizeof(ChunkHeader) + _itemSize * ChunkSize, 4);
    ChunkHeader *allocatedChunk = (ChunkHeader *)chunk;
    *c = allocatedChunk;
    allocatedChunk->Data = (u8 *)(chunk + sizeof(ChunkHeader));

    _nextFree = (BaseComponent *)allocatedChunk->Data;
    for (u32 i = 0; i < ChunkSize - 1; ++i)
    {
        *(u8 **)(allocatedChunk->Data + i * _itemSize) = (allocatedChunk->Data + i * _itemSize + 1);
    }

    *(u8 **)(allocatedChunk->Data + (ChunkSize - 1) * _itemSize) = nullptr;
}
}  // namespace DG
