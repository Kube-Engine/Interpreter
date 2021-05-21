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

    /** @brief Data union */
    using Data = union {
        OperatorType    operatorType { OperatorType::None };
        StatementType   statementType;
        ConstantType    constantType;
    };

    /** @brief An unique pointer using the custom deleter class */
    using Ptr = std::unique_ptr<AST, Deleter>;


    /** @brief Create a new AST node pointer */
    [[nodiscard]] static inline Ptr Make(const Token *token, const TokenType type) noexcept
        { return Ptr(new (Allocate(sizeof(AST), alignof(AST))) AST(token, type)); }

    /** @brief Create a new AST node pointer using a data type */
    template<typename DataType>
    [[nodiscard]] static inline Ptr Make(const Token *token, const TokenType type, const DataType data) noexcept
        { return Ptr(new (Allocate(sizeof(AST), alignof(AST))) AST(token, type, data)); }


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

    /** @brief Get binary type (unsafe if you don't check token type) */
    [[nodiscard]] OperatorType operatorType(void) const noexcept { return _data.operatorType; };

    /** @brief Get statement type (unsafe if you don't check token type) */
    [[nodiscard]] StatementType statementType(void) const noexcept { return _data.statementType; };

    /** @brief Get constant type (unsafe if you don't check token type) */
    [[nodiscard]] ConstantType constantType(void) const noexcept { return _data.constantType; };


    /** @brief Dump the whole tree (debug purposes) */
    void dump(const std::size_t level = 0u, const bool firstOperation = true) const noexcept;

private:
    /** @brief AST node allocator */
    static inline std::pmr::synchronized_pool_resource _Pool {};

    /** @brief Allocate memory using memory pool */
    [[nodiscard]] static void *Allocate(const std::size_t bytes, const std::size_t alignment) noexcept
        { return _Pool.allocate(bytes, alignment); }

    /** @brief Deallocate memory using memory pool */
    static void Deallocate(void * const data, const std::size_t bytes, const std::size_t alignment) noexcept
        { _Pool.deallocate(data, bytes, alignment); }


    /** @brief Constructor */
    AST(const Token *token, const TokenType type) noexcept : _token(token), _type(type) {}

    /** @brief Data constructor */
    template<typename DataType>
    AST(const Token *token, const TokenType type, const DataType data) noexcept : _token(token), _type(type)
    {
        static_assert(
            std::is_same_v<Data, DataType> ||
            std::is_same_v<OperatorType, DataType> ||
            std::is_same_v<StatementType, DataType> ||
            std::is_same_v<ConstantType, DataType>,
            "An AST token must have a valid data field"
        );

        if constexpr (std::is_same_v<OperatorType, DataType>)
            _data.operatorType = data;
        else if constexpr (std::is_same_v<StatementType, DataType>)
            _data.statementType = data;
        else if constexpr (std::is_same_v<ConstantType, DataType>)
            _data.constantType = data;
        else
            _data = data;
    }

    /** @brief Copy constructor */
    AST(const AST &other) noexcept = default;

    /** @brief Move constructor */
    AST(AST &&other) noexcept = default;

private:
    const Token *_token;
    TokenType _type;
    Data _data {};
    Core::AllocatedTinySmallVector<Ptr, 4, &Allocate, &Deallocate> _children {};
};

static_assert_fit_cacheline(kF::Lang::AST);