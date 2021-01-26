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
    /** @brief Token iterator */
    class Iterator
    {
    public:
        /** @brief STL compatibility */
        using iterator_category = std::forward_iterator_tag;
        using value_type = Token;
        using difference_type = std::size_t;
        using pointer = Token *;
        using reference = Token &;

        /** @brief Constructor */
        Iterator(const Token *data) noexcept : _data(data) {}

        /** @brief Copy constructor */
        Iterator(const Iterator &other) noexcept = default;

        /** @brief Destructor */
        ~Iterator(void) noexcept = default;

        /** @brief Copy assignment */
        Iterator &operator=(const Iterator &other) noexcept = default;

        /** @brief Comparison operator */
        [[nodiscard]] bool operator==(const Iterator &other) const noexcept
            { return _data == other._data; }
        [[nodiscard]] bool operator!=(const Iterator &other) const noexcept
            { return _data != other._data; }

        /** @brief Access operators */
        [[nodiscard]] const Token &operator*(void) const noexcept { return *_data; }
        [[nodiscard]] const Token *operator->(void) const noexcept { return _data; }

        /** @brief Retreive token's literal */
        [[nodiscard]] std::string_view literal(void) const noexcept
            { return std::string_view(reinterpret_cast<const char *>(_data) + sizeof(Token), _data->length); }

        /** @brief Prefix Increment operator */
        Iterator &operator++(void) noexcept
        {
            _data = reinterpret_cast<const Token *>(
                reinterpret_cast<const std::byte *>(_data) + _data->length + sizeof(Token)
            );
            return *this;
        }

        /** @brief Sufix Increment operator */
        Iterator operator++(int) noexcept
        {
            auto copy = *this;
            _data = reinterpret_cast<const Token *>(
                reinterpret_cast<const std::byte *>(_data) + _data->length + sizeof(Token)
            );
            return copy;
        }

    private:
        const Token *_data { nullptr };
    };

    /** @brief Insert a token into the cache */
    void push(const Token token, const char * const string) noexcept;

    /** @brief Get token begin for traversal */
    [[nodiscard]] Iterator begin(void) const noexcept
        { return Iterator(reinterpret_cast<const Token *>(_data.begin())); }

    /** @brief Get token end for traversal */
    [[nodiscard]] Iterator end(void) const noexcept
        { return Iterator(reinterpret_cast<const Token *>(_data.end())); }

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
    static inline std::pmr::synchronized_pool_resource _Pool {};

private:
    Core::AllocatedTinyVector<std::byte, &Allocate, &Deallocate> _data {};
};

static_assert_fit_quarter_cacheline(kF::Lang::TokenStack);

#include "TokenStack.ipp"