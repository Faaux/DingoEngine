/**
 *  @file    DG_Type.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include "DG_Include.h"

namespace DG
{
typedef const char* TypeLoc;
typedef TypeLoc* TypeId;

#define STRFY(a) #a
#define DECLARE_CLASS_TYPE(Class, BaseClass)                                                 \
   public:                                                                                   \
    template <class T>                                                                       \
    SDL_FORCE_INLINE bool IsTypeOrSubType() const                                            \
    {                                                                                        \
        const TypeId type = T::GetClassType();                                               \
        return Class::IsType(type) || BaseClass::IsTypeOrSubType(type);                      \
    }                                                                                        \
    SDL_FORCE_INLINE bool IsTypeOrSubType(TypeId type) const override                        \
    {                                                                                        \
        return Class::IsType(type) || BaseClass::IsTypeOrSubType(type);                      \
    }                                                                                        \
    template <class T>                                                                       \
    SDL_FORCE_INLINE bool IsType() const                                                     \
    {                                                                                        \
        return T::GetClassType() == Class::GetInstanceType();                                \
    }                                                                                        \
    SDL_FORCE_INLINE bool IsType(TypeId type) const override                                 \
    {                                                                                        \
        return type == Class::GetInstanceType();                                             \
    }                                                                                        \
    SDL_FORCE_INLINE TypeId GetInstanceType() const override { return &s_myTypeId; }         \
    SDL_FORCE_INLINE static TypeId GetClassType() { return &s_myTypeId; }                    \
    operator TypeId() const { return GetInstanceType(); }                                    \
                                                                                             \
   private:                                                                                  \
    inline static TypeLoc s_myTypeId = STRFY(Class);

class TypeBase
{
protected:
    ~TypeBase() = default;
public:
    template <typename T>
    SDL_FORCE_INLINE bool IsTypeOrSubType() const
    {
        const TypeId type = T::GetClassType();
        return IsType(type) || TypeBase::IsTypeOrSubType(type);
    }
    SDL_FORCE_INLINE virtual bool IsTypeOrSubType(TypeId type) const
    {
        return TypeBase::IsType(type);
    };
    SDL_FORCE_INLINE virtual bool IsType(TypeId type) const
    {
        return type == TypeBase::GetInstanceType();
    }
    SDL_FORCE_INLINE virtual TypeId GetInstanceType() const { return &s_myTypeId; };
    SDL_FORCE_INLINE static TypeId GetClassType() { return &s_myTypeId; }

   private:
    inline static TypeLoc s_myTypeId = STRFY(Base);
};
}  // namespace DG
