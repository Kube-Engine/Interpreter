/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: TokenStack
 */

inline void kF::Lang::TokenStack::push(const Token token, const char * const string) noexcept
{
    _data.insert(_data.end(), reinterpret_cast<const std::byte *>(&token), reinterpret_cast<const std::byte *>(&token + 1));
    _data.insert(_data.end(), reinterpret_cast<const std::byte *>(string), reinterpret_cast<const std::byte *>(string + token.length));
}
