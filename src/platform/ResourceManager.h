/**
 *  @file    ResourceManager.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include <optional>
#include <vector>
#include "StringIdCRC32.h"
#include "engine/Types.h"

namespace DG
{
template <class Key, class T, class Hash = std::hash<Key>, class Pred = std::equal_to<T>>
class HashTable
{
   public:
    class HashTableIterator
    {
        friend class HashTable;
        using vecIterator = typename std::vector<std::optional<T>>::iterator;

       public:
        HashTableIterator operator++()
        {
            if (ptr_ == end_)
                return *this;
            ++ptr_;
            while (ptr_ != end_ && !ptr_->has_value())
            {
                ++ptr_;
            }
            return *this;
        }
        HashTableIterator operator++(int junk)
        {
            if (ptr_ == end_)
                return *this;
            ++ptr_;
            while (ptr_ != end_ && !ptr_->has_value())
            {
                ++ptr_;
            }
            return *this;
        }
        T& operator*() { return *ptr_.value(); }
        T* operator->() { return &ptr_->value(); }
        bool operator==(const HashTableIterator& rhs) { return ptr_ == rhs.ptr_; }
        bool operator!=(const HashTableIterator& rhs) { return ptr_ != rhs.ptr_; }

       private:
        vecIterator ptr_;
        vecIterator end_;
        HashTableIterator(vecIterator ptr, vecIterator end) : ptr_(ptr), end_(end)
        {
            while (ptr_ != end_ && !ptr_->has_value())
            {
                ++ptr_;
            }
        }
    };
    HashTable(u32 tableSize) : _tableSize(tableSize), _values(tableSize) {}

    template <typename... Args>
    T* PutAndConstruct(Key key, Args&&... args);

    T* Put(Key key, const T& value);
    T* Exists(Key key);

    HashTableIterator begin() { return HashTableIterator(_values.begin(), _values.end()); }
    HashTableIterator end() { return HashTableIterator(_values.end(), _values.end()); }

   private:
    u32 _tableSize;
    std::vector<std::optional<T>> _values;
};

template <class Key, class T, class Hash, class Pred>
template <typename... Args>
T* HashTable<Key, T, Hash, Pred>::PutAndConstruct(Key key, Args&&... args)
{
    auto hashedValue = Hash{}(key);
    auto index = hashedValue % _tableSize;
    if (_values[index].has_value())
    {
#if _DEBUG
        // ToDo: Reenable
        // T value(std::forward<Args>(args)...);
        // Assert(Pred{}(_values[index].value(), value));
#endif
    }
    else
    {
        new (&_values[index]) std::optional<T>(std::in_place, std::forward<Args>(args)...);
    }

    return &_values[index].value();
}

template <class Key, class T, class Hash, class Pred>
T* HashTable<Key, T, Hash, Pred>::Put(Key key, const T& value)
{
    auto hashedValue = Hash{}(key);
    auto index = hashedValue % _tableSize;
    if (_values[index].has_value())
    {
        // ToDo: Reenable
        // Assert(Pred{}(_values[index].value(), value));
    }
    else
    {
        _values[index] = value;
    }

    return &_values[index].value();
}

template <class Key, class T, class Hash, class Pred>
T* HashTable<Key, T, Hash, Pred>::Exists(Key key)
{
    auto hashedValue = Hash{}(key);
    auto index = hashedValue % _tableSize;
    return _values[index].has_value() ? &_values[index].value() : nullptr;
}

struct StringIdHasher
{
    std::size_t operator()(const StringId& k) const { return k.GetHash(); }
};

template <typename T>
class ResourceManager
{
   public:
    T* Exists(StringId id);

    auto begin() { return _hashTable.begin(); }
    auto end() { return _hashTable.end(); }

   protected:
    ResourceManager() : _hashTable(1024) {}

    template <typename... Args>
    T* RegisterAndConstruct(StringId id, Args&&... args);
    T* Register(StringId id, const T& value);

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
T* ResourceManager<T>::RegisterAndConstruct(StringId id, Args&&... args)
{
    return _hashTable.PutAndConstruct(id, std::forward<Args>(args)...);
}

template <typename T>
T* ResourceManager<T>::Register(StringId id, const T& value)
{
    return _hashTable.Put(id, value);
}
}  // namespace DG
