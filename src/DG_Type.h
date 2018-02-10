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
#define DECLARE_CLASS_TYPE(Class, BaseClass)                                         \
   public:                                                                           \
    template <class T>                                                               \
    SDL_FORCE_INLINE bool isTypeOrSubType() const                                    \
    {                                                                                \
        static_assert(std::is_base_of<Base, T>::value, "T not derived from Base");   \
        const TypeId type = T::getClassType();                                       \
        return Class::isType(type) || BaseClass::isTypeOrSubType(type);              \
    }                                                                                \
    SDL_FORCE_INLINE bool isTypeOrSubType(TypeId type) const override                \
    {                                                                                \
        return Class::isType(type) || BaseClass::isTypeOrSubType(type);              \
    }                                                                                \
    template <class T>                                                               \
    SDL_FORCE_INLINE bool isType() const                                             \
    {                                                                                \
        static_assert(std::is_base_of<Base, T>::value, "T not derived from Base");   \
        return T::getClassType() == Class::getInstanceType();                        \
    }                                                                                \
    SDL_FORCE_INLINE bool isType(TypeId type) const override                         \
    {                                                                                \
        return type == Class::getInstanceType();                                     \
    }                                                                                \
    SDL_FORCE_INLINE TypeId getInstanceType() const override { return &s_myTypeId; } \
    SDL_FORCE_INLINE static TypeId getClassType() { return &s_myTypeId; }            \
    operator TypeId() const { return getInstanceType(); }                            \
                                                                                     \
   private:                                                                          \
    inline static TypeLoc s_myTypeId = STRFY(Class);

class Base
{
   public:
    virtual ~Base() = default;
    template <typename T>
    SDL_FORCE_INLINE bool isTypeOrSubType() const
    {
        const TypeId type = T::getClassType();
        return A::isType(type) || Base::isTypeOrSubType(type);
    }
    SDL_FORCE_INLINE virtual bool isTypeOrSubType(TypeId type) const { return Base::isType(type); };
    SDL_FORCE_INLINE virtual bool isType(TypeId type) const
    {
        return type == Base::getInstanceType();
    }
    SDL_FORCE_INLINE virtual TypeId getInstanceType() const { return &s_myTypeId; };
    SDL_FORCE_INLINE static TypeId getClassType() { return &s_myTypeId; }

   private:
    inline static TypeLoc s_myTypeId = STRFY(Base);
};
}  // namespace DG
