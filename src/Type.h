/**
 *  @file    Type.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "DG_Include.h"
#include "json.hpp"

namespace DG
{
typedef const char* TypeLoc;
typedef TypeLoc* TypeId;

#ifdef __CODE_GENERATOR__
#define DPROPERTY __attribute__((annotate("DPROPERTY")))
#else
#define DPROPERTY
#endif

#define STRFY(a) #a
#define DECLARE_CLASS_TYPE(Class, BaseClass)                                                      \
    friend nlohmann::json Serialize##Class(const Class* component);                               \
                                                                                                  \
   public:                                                                                        \
    template <class T>                                                                            \
    SDL_FORCE_INLINE bool IsTypeOrDerivedType() const                                             \
    {                                                                                             \
        const TypeId type = T::GetClassType();                                                    \
        return IsTypeOrDerivedType(type);                                                         \
    }                                                                                             \
    SDL_FORCE_INLINE bool IsTypeOrDerivedType(TypeId type) const override                         \
    {                                                                                             \
        return Class::IsTypeInternal(type) || BaseClass::IsTypeOrDerivedType(type);               \
    }                                                                                             \
    SDL_FORCE_INLINE nlohmann::json Serialize() const override { return Serialize##Class(this); } \
                                                                                                  \
   private:                                                                                       \
    SDL_FORCE_INLINE bool IsTypeInternal(TypeId type) const override                              \
    {                                                                                             \
        auto myType = Class::GetInstanceType();                                                   \
        return type == myType;                                                                    \
    }                                                                                             \
                                                                                                  \
   public:                                                                                        \
    SDL_FORCE_INLINE TypeId GetInstanceType() const override { return &s_myTypeId; }              \
    SDL_FORCE_INLINE static TypeId GetClassType() { return &s_myTypeId; }                         \
    operator TypeId() const { return GetInstanceType(); }                                         \
                                                                                                  \
   private:                                                                                       \
    inline static TypeLoc s_myTypeId = STRFY(Class);

class TypeBase
{
   protected:
    ~TypeBase() = default;

   public:
    template <typename T>
    SDL_FORCE_INLINE bool IsTypeOrDerivedType() const
    {
        const TypeId type = T::GetClassType();
        return IsTypeOrDerivedType(type);
    }
    SDL_FORCE_INLINE virtual bool IsTypeOrDerivedType(TypeId type) const
    {
        return TypeBase::IsTypeInternal(type);
    };
    template <class T>
    SDL_FORCE_INLINE bool IsType() const
    {
        return T::GetClassType() == GetInstanceType();
    }
    SDL_FORCE_INLINE virtual bool IsType(TypeId type) const { return type == GetInstanceType(); }
    SDL_FORCE_INLINE virtual TypeId GetInstanceType() const { return &s_myTypeId; };
    SDL_FORCE_INLINE static TypeId GetClassType() { return &s_myTypeId; }

    SDL_FORCE_INLINE virtual nlohmann::json Serialize() const = 0;

   private:
    SDL_FORCE_INLINE virtual bool IsTypeInternal(TypeId type) const
    {
        return type == TypeBase::GetInstanceType();
    }

    inline static TypeLoc s_myTypeId = STRFY(TypeBase);
};
}  // namespace DG
