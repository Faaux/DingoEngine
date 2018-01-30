#include "DG_memory.h"
namespace DG
{
u8* StackAllocator::Push(u32 size, u32 alignment)
{
    Assert(_isInitialized);

    size_t baseAddress = (size_t)(_base + _currentUse);
    size_t padding = (((baseAddress / alignment) + 1) * alignment) - baseAddress;

    Assert(size + padding + _currentUse <= _size);
    Assert(padding >= sizeof(StackHeader));
    Assert(padding <= 255);

    reinterpret_cast<StackHeader*>(baseAddress + padding - sizeof(StackHeader))->padding =
        static_cast<u8>(padding);

    void* result = reinterpret_cast<void*>(baseAddress + padding);
    _currentUse += size + static_cast<u8>(padding);
    _hwm = glm::max(_currentUse, _hwm);
    return static_cast<u8*>(result);
}

void StackAllocator::Pop(void* p)
{
    Assert(_isInitialized);
    Assert(p <= _base + _size);

    u8* ptr = static_cast<u8*>(p);
    u8 padding = reinterpret_cast<StackHeader*>(ptr - 1)->padding;

    u8* newHeadLocation = ptr - padding;

    Assert(newHeadLocation >= _base);
    _currentUse = static_cast<u32>(newHeadLocation - _base);
}

void StackAllocator::Reset() { _currentUse = 0; }
}  // namespace DG
