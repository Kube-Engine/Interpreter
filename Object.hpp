/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Interpreter Object interface
 */

#pragma once

#include "Reflection.hpp"

namespace kF
{
    class Object;
}

class kF::Object
{
    KUBE_REGISTER(Object)

public:
    /** @brief Vector containing list of owned connections */
    using Connections = std::vector<Meta::Connection>;

    /** @brief Move constructor */
    Object(void) = default;
    Object(Object &&other) = default;

    /** @brief Virtual destructor */
    virtual ~Object(void) { disconnect(); }

    /** @brief Checks if a meta-variable exists */
    [[nodiscard]] virtual bool exists(const HashedName name) const noexcept { return getMetaType().findData(name).operator bool(); }

    /** @brief Get an opaque meta-variable */
    [[nodiscard]] virtual Var get(const HashedName name) const;

    /** @brief Get an opaque meta-variable */
    [[nodiscard]] Var get(const Meta::Data metaData) const { return metaData.get(getTypeHandle()); }

    /** @brief Get a casted meta-variable's reference */
    template<typename As>
    [[nodiscard]] As &getAs(const HashedName name) { return get(name).cast<std::remove_reference_t<As>>(); }

    /** @brief Get a casted meta-variable's const reference */
    template<typename As>
    [[nodiscard]] const As &getAs(const HashedName name) const { return get(name).cast<std::remove_reference_t<As>>(); }

    /** @brief Set internal 'name' property */
    virtual void set(const HashedName name, const Var &var);

    /** @brief Set internal 'name' property directly using Meta::Data (faster) */
    void set(const Meta::Data metaData, const Var &var);

    /** @brief Invoke a meta-function */
    template<typename ...Args>
    Var invoke(const HashedName name, Args &&...args);

    /** @brief Invoke a meta-function */
    template<typename ...Args>
    Var invoke(const Meta::Function metaFunc, Args &&...args);

    /**
     * @brief Connect a slot to a signal
     *
     * If the receiver is a Object-derived class, he has the ownership of the connection
     * Else, the calling instance will own it
     */
    template<auto SignalPtr, typename Receiver, typename Slot>
    void connect(const Receiver &receiver, Slot &&slot) noexcept_ndebug { connect(getMetaType().findSignal<SignalPtr>(), receiver, std::forward<Slot>(slot)); }
    template<typename Receiver, typename Slot>
    void connect(const HashedName name, const Receiver &receiver, Slot &&slot) noexcept_ndebug { connect(getMetaType().findSignal(name), receiver, std::forward<Slot>(slot)); }

    /**
     * @brief Connect a slot to a signal, the connection is owned by the calling instance
     *
     * Be careful with with functors as you won't be able to disconnect them manually !
     */
    template<auto SignalPtr, typename Slot>
    void connect(Slot &&slot) noexcept_ndebug { connect<SignalPtr>(nullptr, std::forward<Slot>(slot)); }
    template<typename Slot>
    void connect(const HashedName name, Slot &&slot) noexcept_ndebug { connect(name, nullptr, std::forward<Slot>(slot)); }

    /** @brief Disconnect a member slot of receiver */
    template<auto SignalPtr, typename Receiver, typename Slot>
    void disconnect(const Receiver &receiver, Slot &&slot) noexcept_ndebug { disconnect(getMetaType().findSignal<SignalPtr>(), receiver, std::forward<Slot>(slot)); }
    template<typename Receiver, typename Slot>
    void disconnect(const HashedName name, const Receiver &receiver, Slot &&slot) noexcept_ndebug { disconnect(getMetaType().findSignal(name), receiver, std::forward<Slot>(slot)); }

    /** @brief Disconnect a static slot */
    template<auto SignalPtr, typename Slot>
    void disconnect(Slot &&slot) noexcept_ndebug { disconnect<SignalPtr>(nullptr, std::forward<Slot>(slot)); }
    template<typename Slot>
    void disconnect(const HashedName name, Slot &&slot) noexcept_ndebug { disconnect(name, nullptr, std::forward<Slot>(slot)); }

    /** @brief Disconnect all slots */
    void disconnect(void) noexcept { _connections.clear(); }

    /** @brief Emit signal matching 'SignalPtr' */
    template<auto SignalPtr, typename ...Args>
    void emit(Args &&...args) const;

    /** @brief Emit signal matching 'name' */
    template<typename ...Args>
    void emit(const HashedName name, Args &&...args) const;

private:
    Connections _connections;

    template<typename Receiver, typename Slot>
    void connect(Meta::Signal signal, const Receiver &receiver, Slot &&slot) noexcept_ndebug;
    template<typename Receiver, typename Slot>
    void disconnect(const Meta::Signal signal, const Receiver &receiver, Slot &&slot) noexcept_ndebug;
};

#include "Object.ipp"