/**
 * Inspiration : https://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
 *  @file    Delegate.h
 *  @author  thejelmega ( https://github.com/TheJelmega )
 *  @date    13 February 2018
 */

#pragma once
#include <utility>
#include "engine/Types.h"

// Based on https://codereview.stackexchange.com/questions/14730/impossibly-fast-delegate-in-c11

template <typename T>
class Delegate;

/**
 * Function Delegate
 * @tparam R	Return type
 * @tparam Args	Argument types
 * @note		Delegate has no support for lambdas
 * @note		Ctor for functor needs to use (&functorObject, FunctorType::operator())
 */
template <typename R, typename... Args>
class Delegate<R(Args...)>
{
   private:
    using StubPointer = R (*)(void*, Args&&...);

    template <typename C>
    using DelPair = std::pair<C*, R (C::*)(Args...)>;

    Delegate(void* pObj, StubPointer pStub) : m_pObj(pObj), m_pData(nullptr), m_pStub(pStub) {}

    template <typename C>
    Delegate(const DelPair<C>& pair) : m_pData(nullptr)
    {
        *((DelPair<C>*)&m_pObj) = pair;
        m_pStub = PairStub<DelPair<C>>;
    }

   public:
    /**
     * Create an empty delegate
     */
    Delegate() : m_pObj(nullptr), m_pData(nullptr), m_pStub(nullptr) {}
    /**
     * Create a delegate from an object and member funcion
     */
    template <typename C>
    Delegate(C* pObj, R (C::*pMethod)(Args...))
    {
        *this = From(pObj, pMethod);
    }

    /**
     * Create a delegate from a free functions
     */
    Delegate(R (*pMethod)(Args...)) : m_pData(nullptr)
    {
        m_pObj = pMethod;
        m_pStub = FunctorStub<R (*)(Args...)>;
    }

    /**
     * Create a delegate from another delegate
     * @param[in] del	Delegate
     */
    Delegate(const Delegate& del) : m_pObj(del.m_pObj), m_pData(del.m_pData), m_pStub(del.m_pStub)
    {
    }
    /**
     * Move a delegate into this delegate
     * @param[in] del	Delegate
     */
    Delegate(Delegate&& del) noexcept
        : m_pObj(del.m_pObj), m_pData(del.m_pData), m_pStub(del.m_pStub)
    {
    }

    Delegate& operator=(const Delegate& del)
    {
        Assert(this != &del);
        m_pObj = del.m_pObj;
        m_pData = del.m_pData;
        m_pStub = del.m_pStub;
        return *this;
    }

    Delegate& operator=(Delegate&& del) noexcept
    {
        Assert(this != &del);
        m_pObj = del.m_pObj;
        m_pData = del.m_pData;
        m_pStub = del.m_pStub;
        return *this;
    }

    bool operator==(const Delegate& del) const
    {
        Assert(this != &del);
        return m_pStub == del.m_pStub && m_pObj == del.m_pObj;
    }

    bool operator!=(const Delegate& del) const
    {
        Assert(this != &del);
        return m_pStub != del.m_pStub || m_pObj != del.m_pObj;
    }

    bool operator==(std::nullptr_t) const { return !m_pStub; }

    bool operator!=(std::nullptr_t) const { return m_pStub; }

    explicit operator bool() const { return m_pStub; }

    R operator()(Args... args)
    {
        Assert(m_pStub);
        if (m_pData)
            return m_pStub(&m_pObj, std::forward<Args>(args)...);
        return m_pStub(m_pObj, std::forward<Args>(args)...);
    }

    /**
     * Set the object
     * @param[in] pObj	Object
     * @note			The user is responsible for giving in an object of the correct type
     */
    void SetObject(void* pObj) { m_pObj = pObj; }
    /**
     * Check if the delegate has an object
     * @return	True if the delegate has an object, false otherwise
     */
    bool HasObject() const { return m_pObj; }
    /**
     * Invoke the delegate without any checks
     * @param[in] args	Arguments
     */
    R Invoke(Args... args) { return m_pStub(std::forward<Args>(args)...); }
    /**
     * Try to invoke a function (does <b>NOT</b> check for an object)
     * @return			True if the function can be called
     */
    bool CanInvoke() const { return m_pStub; }
    /**
     * Try to invoke a function (checks for an object)
     * @return			True if the function can be called
     */
    bool CanMemberInvoke() const { return m_pStub && m_pObj; }

    /**
     * Create a delegate from a free pointer
     * @tparam Func	Function pointer
     * @return		Delegate
     */
    template <R (*Func)(Args...)>
    static Delegate From()
    {
        return Delegate(nullptr, &FunctionStub<Func>);
    }
    /**
     * Create a delegate from a member function
     * @tparam C		Class
     * @tparam Func	Member function pointer
     * @return		Delegate
     */
    template <typename C, R (C::*Func)(Args...)>
    static Delegate From()
    {
        return Delegate(nullptr, &MemberStub<C, Func>);
    }
    /**
     * Create a delegate from a member function and an object
     * @tparam C			Class
     * @tparam Func		Member function pointer
     * @param[in] pObj	Object
     * @return			Delegate
     */
    template <typename C, R (C::*Func)(Args...)>
    static Delegate From(C* pObj)
    {
        return Delegate(pObj, &MemberStub<C, Func>);
    }
    /**
     * Create a delegate from a member function and an object
     * @tparam C			Class
     * @tparam Func		Member function pointer
     * @param[in] pObj	Object
     * @return			Delegate
     */
    template <typename C, R (C::*Func)(Args...)>
    static Delegate From(const C& pObj)
    {
        return Delegate(static_cast<C*>(&pObj), &MemberStub<C, Func>);
    }

    /**
     * Swap the contents of 2 delegates
     * @param[in] del	Delegate
     */
    void Swap(Delegate& del)
    {
        std::swap(m_pStub, del.m_pStub);
        std::swap(m_pObj, del.m_pObj);
    }

    // Create use function pointer as param instead of tparam

    template <typename C>
    static Delegate From(C* pObj, R (C::*pMethod)(Args...))
    {
        return DelPair<C>(pObj, pMethod);
    }

    static Delegate From(R (*pMethod)(Args...)) { return pMethod; }

   private:
    template <R (*Func)(Args...)>
    static R FunctionStub(void*, Args&&... args)
    {
        return Func(std::forward<Args>(args)...);
    }

    template <typename C, R (C::*Func)(Args...)>
    static R MemberStub(void* pObj, Args&&... args)
    {
        Assert(pObj);
        return (static_cast<C*>(pObj)->*Func)(std::forward<Args>(args)...);
    }

    template <typename T>
    static R PairStub(void* pObj, Args&&... args)
    {
        T* pair = (T*)pObj;
        return (pair->first->*(pair->second))(std::forward<Args>(args)...);
    }

    template <typename C>
    static R FunctorStub(void* pObj, Args&&... args)
    {
        return (*(C)pObj)(std::forward<Args>(args)...);
    }

    void* m_pObj; /**< pointer to object */
    // vv magic :) vv
    void* m_pData;       /**< Extra data */
    StubPointer m_pStub; /**< Pointer to function stub */
};
