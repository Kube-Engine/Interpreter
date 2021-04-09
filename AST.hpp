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

<<<<<<< HEAD
=======
    /** @brief List of child */
    using Children = Core::AllocatedTinySmallVector<Ptr, 4, &Allocate, &Deallocate>;

>>>>>>> 9aa0487e9517995f30640b65335f4e181ff23aa2

    /** @brief Create a new AST node pointer */
    [[nodiscard]] Ptr Make(const TokenDescriptor &descriptor) noexcept
        { return Ptr(new (Allocate(sizeof(AST), alignof(AST))) AST(descriptor)); }


    /** @brief Destructor */
    ~AST(void) noexcept = default;

<<<<<<< HEAD
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

=======

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
>>>>>>> 9aa0487e9517995f30640b65335f4e181ff23aa2

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
<<<<<<< HEAD
    const Token *_token;
    TokenType _type;
    Core::AllocatedTinySmallVector<Ptr, 4, &Allocate, &Deallocate> _children {};
=======
    TokenDescriptor _desc {};
    Children _children {};

    /** @brief Default constructor */
    AST(const TokenDescriptor &descriptor) noexcept : _desc(descriptor) {}
>>>>>>> 9aa0487e9517995f30640b65335f4e181ff23aa2
};

static_assert_fit_cacheline(kF::Lang::AST);