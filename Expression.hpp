/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Expression
 */

#pragma once

#include <memory_resource>

#include "Instructions.hpp"
#include "TokenStack.hpp"

namespace kF::Lang
{
    class Expression;

    using ExpressionPtr = Expression *;
}

/** @brief An expression is an opaque accesser that implement execution of expressions
 *  You should call 'Construct' static member function to create a pointer to expression
 *
 *  This class is a view that has ownership over itself, 'this' pointer is the beginning of the node data
 *  Once built cannot be nor moved as its pointer is where is the data
 *
 *  Why such an obscure pattern ? Because it removes a level of indirection when calling a slot
 * */
class kF::Lang::Expression
{
public:
    /** @brief Build an expression out of an */
    [[nodiscard]] static inline ExpressionPtr Construct(void);

    /** @brief Release an expression aquired before with 'Construct' */
    [[nodiscard]] static inline void Release(const ExpressionPtr instance) noexcept
        { Deallocate(instance, instance->size()); }

    /** @brief Default destructor, does nothing */
    ~Expression(void) noexcept = default;

    /** @brief Get the self allocation size */
    [[nodiscard]] std::size_t size(void) const noexcept { return _size; }

    /** @brief Execute the expression */
    Var operator()(Var *args) const;

private:
    std::size_t _size { 0u }; // Used to deallocate 'this' instance

    static inline std::pmr::synchronized_pool_resource _Allocator {};

    /** @brief Construct an expression so it occupies a given size in bytes */
    Expression(const std::size_t size) noexcept
        : _size(size) {}


    /** @brief Get the internal node data (itself) as raw data */
    [[nodiscard]] std::byte *rawData(void) noexcept
        { return reinterpret_cast<std::byte *>(this) + kF::Core::CacheLineEighthSize; }
    [[nodiscard]] const std::byte *rawData(void) const noexcept
        { return reinterpret_cast<const std::byte *>(this) + kF::Core::CacheLineEighthSize; }

    /** @brief Get the internal node data (itself) as node */
    [[nodiscard]] Instruction *data(void) noexcept
        { return reinterpret_cast<Instruction *>(rawData()); }
    [[nodiscard]] const Instruction *data(void) const noexcept
        { return reinterpret_cast<const Instruction *>(rawData()); }

    /** @brief Access a node at byte index */
    [[nodiscard]] Instruction *at(const ByteIndex byteIndex) noexcept
        { return reinterpret_cast<Instruction *>(rawData() + byteIndex); }
    [[nodiscard]] const Instruction *at(const ByteIndex byteIndex) const noexcept
        { return reinterpret_cast<const Instruction *>(rawData() + byteIndex); }


    /** @brief Allocate using the allocator */
    [[nodiscard]] static inline void *Allocate(const std::size_t size) noexcept
        { return _Allocator.allocate(size + sizeof(std::size_t), kF::Core::CacheLineSize); }

    /** @brief Allocate using the allocator */
    static inline void Deallocate(void *data, const std::size_t size) noexcept
        { _Allocator.deallocate(data, size + sizeof(std::size_t), kF::Core::CacheLineSize); }
};

#include "Expression.ipp"
