/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Reflection interface for KUBE
 */

#pragma once

#include <Kube/Meta/Meta.hpp>

#include "Make.hpp"

/** @brief Register a type to the reflection system */
#define KUBE_REGISTER(ClassType, ...) \
private: \
    using _MetaType = ClassType; \
protected: \
    [[nodiscard]] virtual kF::Var getTypeHandle(void) noexcept { return kF::Var::Assign(*this); } \
    [[nodiscard]] virtual kF::Var getTypeHandle(void) const noexcept { return kF::Var::Assign(*this); } \
public: \
    [[nodiscard]] virtual kF::Meta::Type getMetaType(void) const noexcept { return kF::Meta::Factory<_MetaType>::Resolve(); } \
    static void RegisterMetadata(void) noexcept { KUBE_REGISTER_TYPE(ClassType, __VA_ARGS__) } \
private:

/** @brief Register a new type to be instantied by Interpreter (must bean Item-derived class) */
#define KUBE_REGISTER_INSTANTIABLE(ClassType, ...) \
    static kF::Item *MakeInstance(void) { return new ClassType(); } \
    KUBE_REGISTER(ClassType, \
        .function<&ClassType::MakeInstance>(kF::MakeInstanceFunctionName) \
        __VA_ARGS__ \
    )

/** @brief Register a new type (already used by 'KUBE_REGISTER' and 'KUBE_REGISTER_INSTANTIABLE') */
#define KUBE_REGISTER_TYPE(ClassType, ...) \
    kF::Meta::Factory<ClassType>().metaRegister(kF::Hash(#ClassType)) __VA_ARGS__ ;

/** @brief Register a base type */
#define KUBE_REGISTER_BASE(BaseType) \
    .base<BaseType>()

/** @brief Register a custom constructor (not default / copy / move) */
#define KUBE_REGISTER_CONSTRUCTOR(...) \
    .constructor<__VA_ARGS__>();

/** @brief Register a property */
#define KUBE_REGISTER_PROPERTY_COPY(PropertyType, name) \
    .data< \
        static_cast<const PropertyType &(_MetaType::*)(void) const noexcept>(&_MetaType::name), \
        static_cast<bool(_MetaType::*)(const PropertyType &)>(&_MetaType::name) \
    >(kF::Hash(#name)) \
    KUBE_REGISTER_SIGNAL(name##Changed)

/** @brief Register a move only property */
#define KUBE_REGISTER_PROPERTY_MOVE(PropertyType, name) \
    .data< \
        static_cast<const PropertyType &(_MetaType::*)(void) const noexcept>(&_MetaType::name), \
        static_cast<bool(_MetaType::*)(PropertyType &&)>(&_MetaType::name) \
    >(kF::Hash(#name)) \
    KUBE_REGISTER_SIGNAL(name##Changed)

/** @brief Register a function */
#define KUBE_REGISTER_FUNCTION(name) \
    .function<&_MetaType::name>(kF::Hash(#name))

/** @brief Register an overloaded function */
#define KUBE_REGISTER_FUNCTION_OVERLOAD(name, FuncType) \
    .function<static_cast<FuncType>(&_MetaType::name)>(kF::Hash(#name))

#define KUBE_REGISTER_SIGNAL(name) \
    .signal<&_MetaType::name>(kF::Hash(#name))

namespace kF
{
    using TypeHandle = Var;

    /** @brief The named used to register Item instantiation helper */
    constexpr HashedName MakeInstanceFunctionName = kF::Hash("__makePtr__");

    /** @brief Registers all base types, returns false if already loaded */
    bool RegisterMetadata(void);
}