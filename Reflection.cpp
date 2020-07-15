/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Reflection interface for KUBE
 */

#include "Object.hpp"
// #include "String.hpp"
// #include "Vector.hpp"
// #include "Item.hpp"

bool kF::RegisterMetadata(void)
{
    static bool initialized = false;

    if (initialized)
        return false;
    initialized = true;
    Meta::RegisterMetadata();
    Object::RegisterMetadata();
    // String::RegisterMetadata();
    // Vector::RegisterMetadata();
    // Item::RegisterMetadata();
    return true;
}
