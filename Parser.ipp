/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Parser
 */

template<kF::Lang::TokenType Type>
inline kF::Lang::AST &kF::Lang::Parser::insertNode(const Token::Iterator it) noexcept
{
    auto ptr = AST::Make(&*it, Type);
    if constexpr (Type == TokenType::Class) {
        if (_root) {
            auto &inserted = _processStack.back()->children().push(std::move(ptr));
            _processStack.push(inserted.get());
            return *inserted;
        } else {
            _root = AST::Make(&*it, Type);
            _processStack.push(_root.get());
            return *_root;
        }
    } else
        return *_processStack.back()->children().push(std::move(ptr));
}

template<kF::Lang::TokenType Type>
inline kF::Lang::AST &kF::Lang::Parser::insertNode(AST &parent, const Token::Iterator it) noexcept
{
    if constexpr (Type == TokenType::Class) {
        auto &inserted = parent.children().push(AST::Make(&*it, Type));
        _processStack.push(inserted.get());
        return *inserted;
    } else
        return *parent.children().push(AST::Make(&*it, Type));
}

template<kF::Lang::TokenType Type, auto DataType>
inline kF::Lang::AST &kF::Lang::Parser::insertNode(AST &parent, const Token::Iterator it) noexcept
{
    if constexpr (Type == TokenType::Class) {
        auto &inserted = parent.children().push(AST::Make(&*it, Type, DataType));
        _processStack.push(inserted.get());
        return *inserted;
    } else
        return *parent.children().push(AST::Make(&*it, Type, DataType));
}

template<bool First>
inline bool kF::Lang::Parser::IsNameChar(const char c) noexcept
{
    if constexpr (First)
        return std::isalpha(c) || c == '_';
    else
        return std::isalnum(c) || c == '_';
}