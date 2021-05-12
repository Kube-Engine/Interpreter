/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Parser
 */

#pragma once

#include <istream>

#include <Kube/Core/Vector.hpp>
#include <Kube/Core/String.hpp>
#include <Kube/Core/AllocatedVector.hpp>
#include <Kube/Core/AllocatedSmallString.hpp>

#include "TokenStack.hpp"
#include "AST.hpp"

namespace kF::Lang
{
    class Parser;
}

/** @brief The Parser is a processing class that takes an input stream and return a TokenStack */
class alignas_double_cacheline kF::Lang::Parser
{
public:
    /** @brief Cache of an operation node */
    struct alignas_quarter_cacheline OperationNode
    {
        const Token *token { nullptr };
        TokenType type { TokenType::None };
        AST::Data data {};
    };

    /** @brief Process the Parser over a input stream */
    [[nodiscard]] AST::Ptr run(const FileIndex file, const TokenStack *stack, const std::string_view &context)
        { prepare(file, stack, context); return std::move(_root); }

private:
    // Cacheline 1
    Core::TinyVector<AST *> _processStack {};
    const TokenStack *_stack { nullptr };
    Token::Iterator _it { nullptr };
    Token::Iterator _end { nullptr };
    AST::Ptr _root {};
    Core::TinyVector<AST *> _nodeStack {};
    // Cacheline 2
    std::string_view _context {};
    Core::TinyVector<Core::TinyString> _imports {};
    Core::TinyVector<OperationNode> _operationStack {};
    std::uint32_t _operationIndex { 0u };
    std::uint32_t _openedParenthesis { 0u };
    FileIndex _file {};

    /** @brief Prepare the instance for the next process */
    void prepare(const FileIndex file, const TokenStack *stack, const std::string_view &context);

    /** @brief Process AST from the token stack */
    void process(void);

    /** @brief Process an import declaration */
    void processImport(void);

    /** @brief Process a class declaration */
    void processClass(void);

    /** @brief Process a class' function */
    void processFunction(void);

    /** @brief Process a class' signal */
    void processSignal(void);

    /** @brief Process a class' property */
    void processProperty(void);

    /** @brief Process a class' event */
    void processEvent(void);

    /** @brief Process a class' assignment */
    void processAssignment(void);

    /** @brief Process a list of parameters */
    void processParameterList(AST &parent);

    /** @brief Process an expression */
    void processExpression(AST &parent, const std::string_view &terminate);

    /** @brief Process a single line expression */
    void processSingleLineExpression(AST &parent);

    /** @brief Process a single token from an expression */
    void processExpressionToken(AST &parent, const std::string_view &literal);

    /** @brief Process if statement from an expression */
    void processIf(AST &parent);

    /** @brief Process while statement from an expression */
    void processWhile(AST &parent);

    /** @brief Process for statement from an expression */
    void processFor(AST &parent);

    /** @brief Process switch statement from an expression */
    void processSwitch(AST &parent);

    /** @brief Process list statement from an expression */
    void processList(AST &parent);

    /** @brief Process local statement from an expression */
    void processLocal(AST &parent);

    /** @brief Process return statement from an expression */
    void processReturn(AST &parent);

    /** @brief Process break statement from an expression */
    void processBreak(AST &parent);

    /** @brief Process continue statement from an expression */
    void processContinue(AST &parent);


    /** @brief Process operation statement from an expression */
    void processOperation(AST &parent, const std::string_view &terminate);

    /** @brief Process a single token from an operation */
    void processOperationToken(const std::string_view &literal);

    /** @brief Try to process an operator token from an operation */
    [[nodiscard]] bool tryProcessOperator(const std::string_view &literal, OperationNode &operationNode);

    /** @brief Try to process a constant token from an operation */
    [[nodiscard]] bool tryProcessConstant(const std::string_view &literal, OperationNode &operationNode);

    /** @brief Build an operation */
    [[nodiscard]] AST::Ptr buildOperation(void);

    /** @brief Build an operator */
    [[nodiscard]] AST::Ptr buildOperator(AST::Ptr lhs, const std::size_t minPrecedence);

    /** @brief Build an operand */
    [[nodiscard]] AST::Ptr buildOperand(void);


    /** @brief Insert a node in higher process node */
    template<TokenType Type>
    AST &insertNode(const Token::Iterator it) noexcept;

    /** @brief Insert a node in another */
    template<TokenType Type>
    AST &insertNode(AST &parent, const Token::Iterator it) noexcept;

    /** @brief Insert a node in another */
    template<TokenType Type, auto DataType>
    AST &insertNode(AST &parent, const Token::Iterator it) noexcept;


    /** @brief Return a well formated error from a token */
    [[nodiscard]] std::string getTokenError(const Token::Iterator it) const noexcept
        { return getTokenError(*it); }
    [[nodiscard]] std::string getTokenError(const Token &token) const noexcept
        { return "At symbol '" + std::string(token.literal()) + "' from " + std::string(_context) + ":l" + std::to_string(token.line) + ":c" + std::to_string(token.column);  }


    /** @brief Check if a token is a valid literal */
    [[nodiscard]] static inline bool IsLiteral(const std::string_view &token) noexcept
        { return token.size() >= 2 && token.front() == '"' && token.back() == '"'; }

    /** @brief Check if a character is valid in a name context */
    template<bool First>
    [[nodiscard]] static inline bool IsNameChar(const char c) noexcept;

    /** @brief Check if a token is a valid name */
    [[nodiscard]] static inline bool IsName(const std::string_view &token) noexcept
        { return token.size() >= 1 && IsNameChar<true>(token.front()) && std::all_of(token.begin(), token.end(), [](const auto c) { return IsNameChar<false>(c); }); }
};

static_assert_fit_double_cacheline(kF::Lang::Parser);

#include "Parser.ipp"