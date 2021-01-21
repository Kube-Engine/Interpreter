/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Abstract Syntax Tree
 */

#pragma once

#include <memory_resource>

#include <Kube/Core/AllocatedSmallVector.hpp>

namespace kF::Lang
{
    class AST;
}

class alignas_cacheline kF::Lang::AST
{
public:
    /** @brief Destruct and deallocate a node using memory pool */
    struct Deleter
    {
        void operator()(AST * const ast) noexcept
            { ast->~AST(); Deallocate(ast, sizeof(AST), alignof(AST)); }
    };

    /** @brief An unique pointer using the custom deleter class */
    using Ptr = std::unique_ptr<AST, Deleter>;

    /** @brief Create a new AST node pointer */
    template<typename ...Args>
    [[nodiscard]] Ptr Make(Args &&...args) noexcept_constructible(AST, Args...)
        { return Ptr(new (Allocate(sizeof(AST), alignof(AST))) AST(std::forward<Args>(args)...)); }


    /** @brief Default constructor */
    AST(void) noexcept = default;

private:
    /** @brief AST node allocator */
    static inline std::pmr::synchronized_pool_resource _Pool {};

    /** @brief Allocate memory using memory pool */
    [[nodiscard]] static void *Allocate(const std::size_t bytes, const std::size_t alignment) noexcept
        { return _Pool.allocate(bytes, alignment); }

    /** @brief Deallocate memory using memory pool */
    static void Deallocate(void * const data, const std::size_t bytes, const std::size_t alignment) noexcept
        { _Pool.deallocate(data, bytes, alignment); }

private:
    Core::AllocatedTinySmallVector<Ptr, 4, &Allocate, &Deallocate> _children {};
};

static_assert_fit_cacheline(kF::Lang::AST);