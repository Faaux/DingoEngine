#pragma once
#include <optional>
#include <vector>
#include "DG_Include.h"
#include "DG_StringId.h"

namespace DG
{
template <class Key, class T, class Hash = std::hash<Key>, class Pred = std::equal_to<T>>
class HashTable
{
   public:
    HashTable(u32 tableSize) : _tableSize(tableSize), _values(tableSize) {}
    template <typename... Args>
    T* Put(Key key, Args&&... args);
    T* Exists(Key key);

   private:
    u32 _tableSize;
    std::vector<std::optional<T>> _values;
};

template <class Key, class T, class Hash, class Pred>
template <typename... Args>
T* HashTable<Key, T, Hash, Pred>::Put(Key key, Args&&... args)
{
    auto hashedValue = Hash{}(key);
    auto index = hashedValue & _tableSize;
    if (_values[index].has_value())
    {
#if DEBUG
        T value(std::forward<Args>(args)...);
        Assert(Pred{}(_values[index].value(), value));
#endif
    }
    else
    {
        _values[index] = ::new (_values[index]) T(std::forward<Args>(args)...);
    }

    return &_values[index].value();
}

template <class Key, class T, class Hash, class Pred>
T* HashTable<Key, T, Hash, Pred>::Exists(Key key)
{
    auto hashedValue = Hash{}(key);
    auto index = hashedValue & _tableSize;
    return _values[index].has_value() ? &_values[index].value() : nullptr;
}

struct StringIdHasher
{
    std::size_t operator()(const StringId& k) const { return k.id; }
};

template <typename T>
class ResourceManager
{
   public:
    T* Exists(StringId id);

   public:
    ResourceManager() : _hashTable(1024) {}
    template <typename... Args>
    T* Register(StringId id, Args&&... args);

   private:
    HashTable<StringId, T, StringIdHasher> _hashTable;
};

template <typename T>
T* ResourceManager<T>::Exists(StringId id)
{
    return _hashTable.Exists(id);
}

template <typename T>
template <class... Args>
T* ResourceManager<T>::Register(StringId id, Args&&... args)
{
    return _hashTable.Put(id, std::forward<Args>(args)...);
}

}  // namespace DG
