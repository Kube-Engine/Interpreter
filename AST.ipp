/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Abstract Syntax Tree
 */

#include <Kube/Core/AllocatedVector.hpp>

template<typename Callback>
void kF::Lang::AST::traverse(Callback &&callback) const noexcept_invocable(Callback, const kF::Lang::AST &)
{
    if (callback(*this)) {
        for (const auto &child : children()) {
            child->traverse(callback);
        }
    }
}