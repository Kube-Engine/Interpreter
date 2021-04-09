/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Abstract Syntax Tree
 */

#pragma once

#include <memory_resource>

#include <Kube/Core/AllocatedSmallVector.hpp>

#include "Base.hpp"

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


    /** @brief Destructor */
    ~AST(void) noexcept = default;

    /** @brief Copy assignment */
    AST &operator=(const AST &other) noexcept = default;

    /** @brief Move assignment */
    AST &operator=(AST &&other) noexcept = default;


    /** @brief Get node's token */
    [[nodiscard]] const Token *token(void) const noexcept { return _token; }

    /** @brief Get node's token type */
    [[nodiscard]] TokenType type(void) const noexcept { return _type; }

    /** @brief Retreive the list of children */
    [[nodiscard]] auto &children(void) noexcept { return _children; }
    [[nodiscard]] const auto &children(void) const noexcept { return _children; }


private:
    /** @brief AST node allocator */
    static inline std::pmr::synchronized_pool_resource _Pool {};

    /** @brief Allocate memory using memory pool */
    [[nodiscard]] static void *Allocate(const std::size_t bytes, const std::size_t alignment) noexcept
        { return _Pool.allocate(bytes, alignment); }

    /** @brief Deallocate memory using memory pool */
    static void Deallocate(void * const data, const std::size_t bytes, const std::size_t alignment) noexcept
        { _Pool.deallocate(data, bytes, alignment); }


    /** @brief Default constructor */
    AST(const Token *token, const TokenType type) noexcept : _token(token), _type(type) {}

    /** @brief Copy constructor */
    AST(const AST &other) noexcept = default;

    /** @brief Move constructor */
    AST(AST &&other) noexcept = default;

private:
    const Token *_token;
    TokenType _type;
    Core::AllocatedTinySmallVector<Ptr, 4, &Allocate, &Deallocate> _children {};
};

static_assert_fit_cacheline(kF::Lang::AST);