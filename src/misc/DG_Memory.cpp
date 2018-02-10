#include "DG_memory.h"
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
    SDL_memset4(data, 0xDEADBEEF, header->size);
#endif

    // Pop everything off that was freed
    header = (StackHeader*)_current;
    while (header->free)
    {
        _current = _current + (header->size + sizeof(StackHeader));
        header = (StackHeader*)_current;
    }

    Assert(_base + _size >= _current);
}

void StackAllocator::Reset() { _current = _base + _size; }
}  // namespace DG
