/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Interpreter Object interface
 */

#include "Object.hpp"

using namespace kF;

Var Object::get(const HashedName name) const
{
    if (auto data = getMetaType().findData(name); !data)
        throw std::logic_error("Object::get: Invalid hashed name '" + std::to_string(name) + '\'');
    else if (auto res = data.get(getTypeHandle()); res)
        return res;
    throw std::logic_error("Object::get: Empty property '" + std::to_string(name) + '\'');
}

void Object::set(const HashedName name, const Var &var)
{
    if (auto data = getMetaType().findData(name); !data)
        throw std::logic_error("Object::set: Invalid hashed name '" + std::to_string(name) + '\'');
    else if (!data.set(getTypeHandle(), const_cast<Var &>(var)))
        throw std::logic_error("Object::set: Argument type doesn't match type of hashed name '" + std::to_string(name) + '\'');
}

void Object::set(const Meta::Data metaData, const Var &var)
{
    if (!metaData.set(getTypeHandle(), const_cast<Var &>(var)))
        throw std::logic_error("Object::set: Argument type doesn't match type");
}