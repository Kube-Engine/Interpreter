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

    /** @brief List of child */
    using Children = Core::AllocatedTinySmallVector<Ptr, 4, &Allocate, &Deallocate>;


    /** @brief Create a new AST node pointer */
    [[nodiscard]] Ptr Make(const TokenDescriptor &descriptor) noexcept
        { return Ptr(new (Allocate(sizeof(AST), alignof(AST))) AST(descriptor)); }


    /** @brief Destructor */
    ~AST(void) noexcept = default;


    /** @brief Get the token iterator */
    [[nodiscard]] Token::Iterator token(void) const noexcept { return _desc.token; }

    /** @brief Get the token literal */
    [[nodiscard]] std::string_view literal(void) const noexcept { return _desc.token.literal(); }


    /** @brief Get the token type */
    [[nodiscard]] TokenType type(void) const noexcept { return _desc.type; }

    /** @brief Get the unary type, valid if the node is of type unary */
    [[nodiscard]] UnaryType unaryType(void) const noexcept { return _desc.data.unaryType; }

    /** @brief Get the binary type, valid if the node is of type binary */
    [[nodiscard]] BinaryType binaryType(void) const noexcept { return _desc.data.binaryType; }

    /** @brief Get the assignment type, valid if the node is of type assignment */
    [[nodiscard]] AssignmentType assignmentType(void) const noexcept { return _desc.data.assignmentType; }

    /** @brief Get the statement type, valid if the node is of type statement */
    [[nodiscard]] StatementType statementType(void) const noexcept { return _desc.data.statementType; }


    /** @brief Get children list */
    [[nodiscard]] Children &children(void) noexcept { return _children; }
    [[nodiscard]] const Children &children(void) const noexcept { return _children; }

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
    TokenDescriptor _desc {};
    Children _children {};

    /** @brief Default constructor */
    AST(const TokenDescriptor &descriptor) noexcept : _desc(descriptor) {}
};

static_assert_fit_cacheline(kF::Lang::AST);