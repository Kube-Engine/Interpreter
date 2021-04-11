/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Parser
 */

#include <iostream>

#include "Parser.hpp"

using namespace kF;

void Lang::Parser::prepare(const FileIndex file, const TokenStack *stack, const std::string_view &context)
{
    _stack = stack;
    _it = _stack->begin();
    _end = _stack->end();
    _processStack.clear();
    _nodeStack.clear();
    _root.reset();
    _nodeStack.push(_root.get());
    _imports.clear();
    _context = context;
    _file = file;
    process();
}

void Lang::Parser::process(void)
{
    while (_it != _end) {
        const auto literal = _it.literal();
        if (literal == "import")
            processImport();
        else if (IsName(literal))
            processClass();
        else
            throw std::logic_error("Lang::Parser::process: Unexpected token at global scope\n" + getTokenError(_it));
    }
    if (!_root)
        throw std::logic_error("Lang::Parser::process: No class declaration in file '" + std::string(_context) + '\'');
}

void Lang::Parser::processImport(void)
{
    const auto rootIt = _it;

    if (++_it == _end) [[unlikely]]
        throw std::logic_error("Lang::Parser::processImport: Unexpected end of file in import declaration\n" + getTokenError(rootIt));
    const auto literal = _it.literal();
    if (!IsLiteral(literal)) [[unlikely]]
        throw std::logic_error("Lang::Parser::processImport: Import declaration token is not a literal\n" + getTokenError(_it));
    _imports.push(literal);
    ++_it;
}

void Lang::Parser::processClass(void)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processClass: Unexpected end of file in class declaration\n";
    static const char *UnexpectedToken = "Lang::Parser::processClass: Unexpected token in class declaration\n";

    const auto rootIt = _it;

    if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    else if (_it.literal() != "{") [[unlikely]]
        throw std::logic_error(UnexpectedToken + getTokenError(_it));
    else if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    insertNode<TokenType::Class>(rootIt);
    while (_it != _end) {
        const auto literal = _it.literal();
        if (literal == "function")
            processFunction();
        else if (literal == "signal")
            processSignal();
        else if (literal == "property")
            processProperty();
        else if (literal == "on")
            processEvent();
        else if (IsName(literal)) {
            auto next = _it;
            if (++next == _end) [[unlikely]]
                throw std::logic_error(UnexpectedToken + getTokenError(_it));
            const auto literal = next.literal();
            if (literal == ":") [[likely]]
                processAssignment();
            else if (literal == "{")
                processClass();
            else [[unlikely]]
                throw std::logic_error(UnexpectedToken + getTokenError(next));
        } else if (literal == "}") [[unlikely]] {
            ++_it;
            _processStack.pop();
            return;
        } else
            throw std::logic_error(UnexpectedToken + getTokenError(_it));
    }
    throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
}

void Lang::Parser::processFunction(void)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processFunction: Unexpected end of file in function declaration\n";
    static const char *UnexpectedToken = "Lang::Parser::processFunction: Unexpected token in function parameters declaration\n";

    if (const auto rootIt = _it; ++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    const auto nameIt = _it;
    const auto nameLiteral = nameIt.literal();
    auto &rootNode = insertNode<TokenType::Function>(nameIt);

    if (!IsName(nameLiteral)) [[unlikely]]
        throw std::logic_error("Lang::Parser::processFunction: Invalid function name\n" + getTokenError(nameIt));
    else if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(nameIt));
    else if (_it.literal() != "(") [[unlikely]]
        throw std::logic_error(UnexpectedToken + getTokenError(nameIt));
    processParameterList(rootNode);
    if (_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(nameIt));
    else if (_it.literal() != "{") [[unlikely]]
        throw std::logic_error(UnexpectedToken + getTokenError(nameIt));
    processExpression(rootNode, "}");
}

void Lang::Parser::processSignal(void)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processSignal: Unexpected end of file in signal declaration\n";
    static const char *UnexpectedToken = "Lang::Parser::processSignal: Unexpected token in signal declaration\n";

    if (const auto rootIt = _it; ++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    const auto nameIt = _it;
    const auto nameLiteral = nameIt.literal();
    auto &rootNode = insertNode<TokenType::Signal>(nameIt);

    if (!IsName(nameLiteral)) [[unlikely]]
        throw std::logic_error("Lang::Parser::processSignal: Invalid signal name\n" + getTokenError(nameIt));
    else if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(nameIt));
    else if (_it.literal() != "(") [[unlikely]]
        throw std::logic_error(UnexpectedToken + getTokenError(nameIt));
    processParameterList(rootNode);
    if (_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(nameIt));
    else if (_it.literal() != ";") [[unlikely]]
        throw std::logic_error("Lang::Parser::processSignal: Signal declaration must end with a ';'" + getTokenError(nameIt));
    ++_it;
}

void Lang::Parser::processProperty(void)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processProperty: Unexpected end of file in property declaration\n";
    static const char *UnexpectedToken = "Lang::Parser::processProperty: Unexpected token in property declaration\n";

    const auto rootIt = _it;
    auto &rootNode = insertNode<TokenType::Property>(rootIt);

    if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    const auto nameIt = _it;
    const auto nameLiteral = nameIt.literal();
    if (!IsName(nameLiteral)) [[unlikely]]
        throw std::logic_error("Lang::Parser::processProperty: Invalid property name\n" + getTokenError(nameIt));
    else if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(nameIt));
    else if (_it.literal() != ":") [[unlikely]]
        throw std::logic_error(UnexpectedToken + getTokenError(nameIt));
    else if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(nameIt));
    if (_it.literal() == "{")
        processExpression(rootNode, "}");
    else
        processSingleLineExpression(rootNode);
}

void Lang::Parser::processEvent(void)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processEvent: Unexpected end of file in event declaration\n";
    static const char *UnexpectedToken = "Lang::Parser::processEvent: Unexpected token in event declaration\n";

    const auto rootIt = _it;
    auto &rootNode = insertNode<TokenType::Event>(rootIt);

    if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    processExpression(rootNode, ":");
    if (_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    if (_it.literal() == "{")
        processExpression(rootNode, "}");
    else
        processSingleLineExpression(rootNode);
}

void Lang::Parser::processAssignment(void)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processAssignment: Unexpected end of file in assignment declaration\n";
    static const char *UnexpectedToken = "Lang::Parser::processAssignment: Unexpected token in assignment declaration\n";

    const auto rootIt = _it;

    if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    else if (_it.literal() != ":") [[unlikely]]
        throw std::logic_error(UnexpectedToken + getTokenError(_it));
    else if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    auto &rootNode = insertNode<TokenType::Assignment>(rootIt);
    if (_it.literal() == "{")
        processExpression(rootNode, "}");
    else
        processSingleLineExpression(rootNode);
}

void Lang::Parser::processParameterList(AST &parent)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processParameterList: Unexpected end of file in parameter list\n";
    static const char *UnexpectedToken = "Lang::Parser::processParameterList: Unexpected token in parameter list\n";

    const auto rootIt = _it;
    auto &rootNode = insertNode<TokenType::ParameterList>(parent, rootIt);

    if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    while (_it != _end) {
        const auto literal = _it.literal();
        if (literal == ")") [[unlikely]] {
            ++_it;
            return;
        } else if (IsName(literal)) [[likely]] {
            insertNode<TokenType::Name>(rootNode, _it);
            if (++_it == _end) [[unlikely]]
                throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
            if (auto nextLiteral = _it.literal(); nextLiteral == ",") {
                ++_it;
            } else if (nextLiteral != ")") [[unlikely]]
                throw std::logic_error(UnexpectedToken + getTokenError(rootIt));
        } else [[unlikely]]
            throw std::logic_error(UnexpectedToken + getTokenError(rootIt));
    }
    throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
}

void Lang::Parser::processExpression(AST &parent, const std::string_view &terminate)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processExpression: Unexpected end of file in expression\n";
    static const char *UnexpectedToken = "Lang::Parser::processExpression: Unexpected token in expression\n";

    const auto rootIt = _it;
    auto &rootNode = insertNode<TokenType::Expression>(parent, rootIt);

    if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    while (_it != _end) {
        const auto literal = _it.literal();
        if (literal == terminate) [[unlikely]] {
            ++_it;
            return;
        }
        processExpressionToken(rootNode, literal);
        ++_it;
    }
    throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
}

void Lang::Parser::processSingleLineExpression(AST &parent)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processSingleLineExpression: Unexpected end of file in expression\n";

    const auto rootIt = _it;
    const auto line = rootIt->line;
    auto &rootNode = insertNode<TokenType::Expression>(parent, rootIt);

    while (_it != _end) {
        const auto literal = _it.literal();
        if (_it->line != line) [[unlikely]]
            return;
        processExpressionToken(rootNode, literal);
    }
    throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
}

void Lang::Parser::processExpressionToken(AST &parent, const std::string_view &literal)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processExpressionToken: Unexpected end of file in expression\n";

    if (literal == "if")
        processIf(parent);
    else if (literal == "while")
        processWhile(parent);
    else if (literal == "for")
        processFor(parent);
    else if (literal == "switch")
        processSwitch(parent);
    else if (literal == "{")
        processExpression(parent, "}");
    else if (literal == "[")
        processList(parent);
    else if (IsName(literal)) {
        auto next = _it;
        if (++next == _end) [[unlikely]]
            throw std::logic_error(UnexpectedEndOfFile + getTokenError(_it));
        else if (const auto nextLiteral = next.literal(); IsName(nextLiteral) || nextLiteral == "<")
            processLocal(parent);
        else
            processOperation(parent, ";");
    } else
        processOperation(parent, ";");
}

void kF::Lang::Parser::processIf(AST &parent)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processIf: Unexpected end of file in if statement\n";
    static const char *UnexpectedToken = "Lang::Parser::processIf: Unexpected token in if statement\n";

    const auto rootIt = _it;
    auto &rootNode = insertNode<TokenType::Statement, StatementType::If>(parent, rootIt);

    const auto parseIf = [this, &rootNode] {
        const auto rootIt = _it;
        if (++_it == _end) [[unlikely]]
            throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
        else if (_it.literal() != "(")
            throw std::logic_error(UnexpectedToken + getTokenError(rootIt));
        else if (++_it == _end) [[unlikely]]
            throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
        processOperation(rootNode, ")");
        if (_it == _end) [[unlikely]]
            throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
        else if (_it.literal() == "{")
            processExpression(rootNode, "}");
        else
            processSingleLineExpression(rootNode);
    };

    parseIf();
    while (_it != _end) {
        if (_it.literal() == "else") {
            if (++_it == _end) [[unlikely]]
                throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
            else if (const auto literal = _it.literal(); literal == "if")
                parseIf();
            else if (literal == "{") {
                processExpression(rootNode, "}");
            } else
                processSingleLineExpression(rootNode);
        } else
            return;
    }
    throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
}

void kF::Lang::Parser::processWhile(AST &parent)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processWhile: Unexpected end of file in while statement\n";
    static const char *UnexpectedToken = "Lang::Parser::processWhile: Unexpected token in while statement\n";

    const auto rootIt = _it;
    auto &rootNode = insertNode<TokenType::Statement, StatementType::While>(parent, rootIt);

    if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    else if (_it.literal() != "(")
        throw std::logic_error(UnexpectedToken + getTokenError(rootIt));
    else if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    processOperation(rootNode, ")");
    if (_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    else if (_it.literal() == "{")
        processExpression(rootNode, "}");
    else
        processSingleLineExpression(rootNode);
}

void kF::Lang::Parser::processFor(AST &parent)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processFor: Unexpected end of file in for statement\n";
    static const char *UnexpectedToken = "Lang::Parser::processFor: Unexpected token in for statement\n";

    const auto rootIt = _it;
    auto &rootNode = insertNode<TokenType::Statement, StatementType::For>(parent, rootIt);

    if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    else if (_it.literal() != "(")
        throw std::logic_error(UnexpectedToken + getTokenError(rootIt));
    else if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    processOperation(rootNode, ";");
    if (_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    processOperation(rootNode, ";");
    if (_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    processOperation(rootNode, ")");
    if (_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    else if (_it.literal() == "{")
        processExpression(rootNode, "}");
    else
        processSingleLineExpression(rootNode);
}

void kF::Lang::Parser::processSwitch(AST &parent)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processSwitch: Unexpected end of file in switch statement\n";
    static const char *UnexpectedToken = "Lang::Parser::processSwitch: Unexpected token in switch statement\n";

    const auto rootIt = _it;
    auto &rootNode = insertNode<TokenType::Statement, StatementType::Switch>(parent, rootIt);

    if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    else if (_it.literal() != "(")
        throw std::logic_error(UnexpectedToken + getTokenError(_it));
    else if (++_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    processOperation(rootNode, ")");
    if (_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
    else if (_it.literal() != "{")
        throw std::logic_error(UnexpectedToken + getTokenError(_it));
    while (_it != _end) {
        const auto literal = _it.literal();
        if (literal == "}") [[unlikely]] {
            ++_it;
            return;
        } else if (literal == "case") {
            if (++_it == _end) [[unlikely]]
                throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
            processOperation(rootNode, ":");
            if (_it == _end) [[unlikely]]
                throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
            else if (_it.literal() == "{")
                processExpression(rootNode, "}");
            else
                processSingleLineExpression(rootNode);
        } else if (literal == "default") {
            if (++_it == _end) [[unlikely]]
                throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
            else if (_it.literal() != ":")
               throw std::logic_error(UnexpectedToken + getTokenError(_it));
            if (++_it == _end) [[unlikely]]
                throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
            else if (_it.literal() == "{")
                processExpression(rootNode, "}");
            else
                processSingleLineExpression(rootNode);
            return;
        } else
           throw std::logic_error(UnexpectedToken + getTokenError(_it));
    }
    throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
}

void kF::Lang::Parser::processList(AST &parent)
{

}

void kF::Lang::Parser::processLocal(AST &parent)
{

}

void kF::Lang::Parser::processOperation(AST &parent, const std::string_view &terminate)
{
    static const char *UnexpectedEndOfFile = "Lang::Parser::processOperation: Unexpected end of file in operation\n";
    static const char *UnexpectedToken = "Lang::Parser::processOperation: Unexpected token in operation\n";

    const auto rootIt = _it;

    while (_it != _end) {
        const auto literal = _it.literal();
        if (literal != terminate) [[likely]] {
            processOperationToken();
        } else {
            buildOperation(parent, rootIt);
            ++_it;
            return;
        }
    }
    if (_it == _end) [[unlikely]]
        throw std::logic_error(UnexpectedEndOfFile + getTokenError(rootIt));
}

void kF::Lang::Parser::processOperationToken(void)
{
    const auto literal = _it.literal();
    OperationNode operationNode {
        token: &*_it
    };

    if (literal == "(")
        operationNode.type = TokenType::LeftParenthesis;
    else if (literal == ")")
        operationNode.type = TokenType::RightParenthesis;
    else if (tryProcessOperator(literal, operationNode)) {
    } else if (IsName(literal))
        operationNode.type = TokenType::Name;
    else
        throw std::logic_error("Lang::Parser::processOperationToken: Unexpected token in operation\n" + getTokenError(_it));
    _operationStack.push(operationNode);
    ++_it;
}

bool kF::Lang::Parser::tryProcessOperator(const std::string_view &literal, OperationNode &operationNode)
{
    if (literal.empty())
        return false;
    return true;
    // switch (literal.front()) {
    // case '?':
    // case ':':
    // case ',':
    // case '.':
    // // Composed operators with '='
    // case '=':
    // case '<':
    // case '>':
    // case '!':
    // case '*':
    // case '/':
    // case '%':
    //     processComposedSpecialToken<'='>(begin);
    //     return ProcessState::Success;
    // // Custom composed operators
    // case '|':
    //     processComposedSpecialToken<'|', '='>(begin);
    //     return ProcessState::Success;
    // case '+':
    //     processComposedSpecialToken<'+', '='>(begin);
    //     return ProcessState::Success;
    // case '-':
    //     processComposedSpecialToken<'-', '='>(begin);
    //     return ProcessState::Success;
    // // Custom cases

    // case '"':
    //     if (parseString()) [[likely]]
    //         return ProcessState::Success;
    //     else [[unlikely]]
    //         return ProcessState::Error;
    // case '\'':
    //     if (parseCharacter()) [[likely]]
    //         return ProcessState::Success;
    //     else [[unlikely]]
    //         return ProcessState::Error;
    // default:
    //     return ProcessState::NotRecognized;
    // }
}

void kF::Lang::Parser::buildOperation(AST &parent, const Token::Iterator rootIt)
{

}