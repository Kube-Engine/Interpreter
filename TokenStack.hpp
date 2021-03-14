/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: TokenStack
 */

#pragma once

#include <memory_resource>

#include <Kube/Core/AllocatedVector.hpp>
#include <Kube/Core/AllocatedFlatString.hpp>

#include "Base.hpp"

namespace kF::Lang
{
    class TokenStack;
}

class alignas_quarter_cacheline kF::Lang::TokenStack
{
public:
    [[nodiscard]] Token &at(const TokenIndex index) noexcept
        { return reinterpret_cast<Token &>(_data[index]); }
    [[nodiscard]] const Token &at(const TokenIndex index) const noexcept
        { return reinterpret_cast<const Token &>(_data[index]); }

    /** @brief Insert a token into the cache */
    void push(const Token token, const char * const string) noexcept;

    /** @brief Get token begin for traversal */
    [[nodiscard]] Token::Iterator begin(void) const noexcept
        { return Token::Iterator(reinterpret_cast<const Token *>(_data.begin())); }

    /** @brief Get token end for traversal */
    [[nodiscard]] Token::Iterator end(void) const noexcept
        { return Token::Iterator(reinterpret_cast<const Token *>(_data.end())); }

    /** @brief Release all owned memory */
    void release(void) { _data.release(); }


public: // Allocator static members
    /** @brief Allocate from the pool */
    [[nodiscard]] static inline void *Allocate(const std::size_t bytes, const std::size_t alignment) noexcept
        { return _Pool.allocate(bytes, alignment); }

    /** @brief Deallocate from the pool */
    static inline void Deallocate(void * const data, const std::size_t bytes, const std::size_t alignment) noexcept
        { _Pool.deallocate(data, bytes, alignment); }

    /** @brief Release the whole pool memory
     *  ! You must ensure that all TokenStack are released ! */
    static inline void Release(void) { _Pool.release(); }

private:
    Core::AllocatedTinyVector<std::byte, &Allocate, &Deallocate> _data {};

    static inline std::pmr::synchronized_pool_resource _Pool {};
};

static_assert_fit_quarter_cacheline(kF::Lang::TokenStack);

#include "TokenStack.ipp"