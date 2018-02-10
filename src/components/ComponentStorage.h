/**
 *  @file    ComponentStorage.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "BaseComponent.h"
#include "DG_Include.h"
#include "DG_Memory.h"

namespace DG
{
class BaseComponentStorage
{
    enum
    {
        ChunkSize = 64
    };

   public:
    void ReleaseComponent(BaseComponent *component);

   protected:
    BaseComponentStorage(StackAllocator *allocator, u32 itemSize);

    void *CreateComponent();

   private:
    void AllocateNewChunk();

    struct ChunkHeader
    {
        ChunkHeader *Next;
        u8 *Data;
    };

    static_assert(sizeof(ChunkHeader) % 4 == 0);  // To not loose alignment for data

    // ToDo: Make this grow as needed!
    u32 _itemSize;
    StackAllocator *_allocator;
    ChunkHeader *_baseChunk = nullptr;
    BaseComponent *_nextFree = nullptr;
};
template <class T>
class ComponentStorage : public BaseComponentStorage
{
    // static_assert(std::is_base_of<BaseComponent<T>, T>::value, "T not derived from
    // BaseComponent");
    static_assert(sizeof(T *) <= sizeof(T));

   public:
    ComponentStorage(StackAllocator *allocator);

    T *CreateComponent();
};

template <class T>
ComponentStorage<T>::ComponentStorage(StackAllocator *allocator)
    : BaseComponentStorage(allocator, sizeof(T))
{
}

template <class T>
T *ComponentStorage<T>::CreateComponent()
{
    return reinterpret_cast<T *>(BaseComponentStorage::CreateComponent());
}

}  // namespace DG
