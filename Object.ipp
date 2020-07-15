/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Interpreter Object interface
 */

template<typename ...Args>
kF::Var kF::Object::invoke(const HashedName name, Args &&...args)
{
    auto handle = getTypeHandle();
    auto fct = handle.type().findFunction(name);

    kFAssert(fct,
        throw std::logic_error("Object::invoke: Invalid hashed name '" + std::to_string(name) + '\''));
    return fct.invoke(handle, std::forward<Args>(args)...);
}

template<typename ...Args>
kF::Var kF::Object::invoke(const Meta::Function metaFunc, Args &&...args)
{
    return metaFunc.invoke(getTypeHandle(), std::forward<Args>(args)...);
}

template<typename Receiver, typename Slot>
void kF::Object::connect(Meta::Signal signal, const Receiver &receiver, Slot &&slot) noexcept_ndebug
{
    kFAssert(signal,
        throw std::logic_error("Object::connect: Invalid signal"));
    if constexpr (std::is_same_v<Receiver, std::nullptr_t>)
        _connections.emplace_back(signal.connect(*this, std::forward<Slot>(slot)));
    else if constexpr (std::is_base_of_v<Object, Receiver>)
        receiver->_connections.emplace_back(signal.connect(*this, receiver, std::forward<Slot>(slot)));
    else
        _connections.emplace_back(signal.connect(*this, receiver, std::forward<Slot>(slot)));
}

template<typename Receiver, typename Slot>
void kF::Object::disconnect(const Meta::Signal signal, const Receiver &receiver, Slot &&slot) noexcept_ndebug
{
    Connections *connections;

    if constexpr (std::is_base_of_v<Object, Receiver>)
        connections = &receiver->_connections;
    else
        connections = _connections;
    for (auto it = connections->begin(), end = connections->end(); it != end; ++it) {
        if (it->signal() != signal ||
                it->sender() != reinterpret_cast<const void *>(this) ||
                it->opaqueFunctor()->receiver != reinterpret_cast<const void *>(&receiver) ||
                it->opaqueFunctor()->data.type() != Meta::Factory<Slot>::Resolve() ||
                it->opaqueFunctor()->data.as<Slot>() != slot)
            continue;
        connections->erase(it);
        return;
    }
    throw std::logic_error("Object::disconnect: Signal doesn't exists");
}

template<auto SignalPtr, typename ...Args>
void kF::Object::emit(Args &&...args) const
{
    auto signal = getMetaType().findSignal<SignalPtr>();

    kFAssert(signal,
        throw std::logic_error("Object::emit: Unknown signal name"));
    signal.emit(this, std::forward<Args>(args)...);
}

template<typename ...Args>
void kF::Object::emit(const HashedName name, Args &&...args) const
{
    auto signal = getMetaType().findSignal(name);

    kFAssert(signal,
        throw std::logic_error("Object::emit: Unknown signal name"));
    signal.emit(this, std::forward<Args>(args)...);
}