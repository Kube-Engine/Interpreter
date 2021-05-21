/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Abstract Syntax Tree
 */

#include <iostream>

#include "AST.hpp"

using namespace kF;

void Lang::AST::dump(const std::size_t level, const bool firstOperation) const noexcept
{
    constexpr auto Tabify = [](const std::size_t level) {
        std::cout << std::string(level * 2, ' ');
    };

    constexpr auto PrintEndOfExpression = [](const AST::Ptr &node) {
        if (const auto type = node->type(); (type == TokenType::Expression && node->children().size() == 1
                && (node->children()[0]->type() != TokenType::Statement || static_cast<std::uint32_t>(node->children()[0]->statementType()) >= static_cast<std::uint32_t>(StatementType::Break)))
                || (static_cast<std::uint32_t>(type) > static_cast<std::uint32_t>(TokenType::Expression)))
            std::cout << ';' << std::endl;
        else
            std::cout << std::endl;
    };

    switch (_type) {
    case TokenType::None:
        std::cout << "NONE" << std::endl;
        break;
    case TokenType::Class:
        std::cout << _token->literal() << " {" << std::endl;
        for (const auto &child : children()) {
            Tabify(level + 1);
            child->dump(level + 1);
        }
        Tabify(level);
        std::cout << '}' << std::endl;;
        break;
    case TokenType::Property:
        std::cout << "property " << _token->literal() << ": ";
        children()[0]->dump(level);
        PrintEndOfExpression(children()[0]);
        break;
    case TokenType::Signal:
        std::cout << "signal " << _token->literal();
        children()[0]->dump(level);
        PrintEndOfExpression(children()[0]);
        break;
    case TokenType::Function:
        std::cout << "function " << _token->literal();
        children()[0]->dump(level);
        std::cout << " ";
        children()[1]->dump(level);
        PrintEndOfExpression(children()[1]);
        break;
    case TokenType::Event:
        std::cout << "on ";
        children()[0]->dump(level);
        std::cout << ": ";
        children()[1]->dump(level);
        PrintEndOfExpression(children()[1]);
        break;
    case TokenType::Assignment:
        std::cout << _token->literal() << ": ";
        children()[0]->dump(level);
        PrintEndOfExpression(children()[0]);
        break;
    case TokenType::ParameterList:
        std::cout << "(";
        for (bool passed = false; const auto &child : children()) {
            if (passed)
                std::cout << ", ";
            else
                passed = true;
            child->dump(level);
        }
        std::cout << ")";
        break;
    case TokenType::Expression:
        if (children().empty())
            std::cout << "{}";
        else if (children().size() == 1u && (children()[0]->type() != TokenType::Statement
                || static_cast<std::uint32_t>(children()[0]->statementType()) >= static_cast<std::uint32_t>(StatementType::Break))) {
            children()[0]->dump(level);
        } else {
            std::cout << '{' << std::endl;
            for (const auto &child : children()) {
                Tabify(level + 1);
                child->dump(level + 1);
                if (child->type() != TokenType::Statement || static_cast<std::uint32_t>(child->statementType()) >= static_cast<std::uint32_t>(StatementType::Break))
                    std::cout << ';' << std::endl;
            }
            Tabify(level);
            std::cout << '}';
        }
        break;
    case TokenType::Name:
        std::cout << _token->literal();
        break;
    case TokenType::List:
        std::cout << "[ ";
        for (bool passed = false; const auto &child : children()) {
            if (passed)
                std::cout << ", ";
            else
                passed = true;
            child->dump(level);
        }
        std::cout << " ]";
        break;
    case TokenType::Local:
        children()[0]->dump(level);
        std::cout << " ";
        children()[1]->dump(level);
        std::cout << " = ";
        children()[2]->dump(level);
        break;
    case TokenType::Type:
        std::cout << _token->literal();
        break;
    case TokenType::Statement:
        switch (_data.statementType) {
        case StatementType::None:
            std::cout << "STATEMENT ERROR" << std::endl;
            break;
        case StatementType::If:
            std::cout << "if (";
            children()[0]->dump(level);
            std::cout << ") ";
            children()[1]->dump(level);
            std::cout << std::endl;
            if (children().size() > 2) {
                for (auto i = 2u; i < children().size();) {
                    Tabify(level);
                    if (i + 1 < children().size()) {
                        std::cout << "else if (";
                        children()[i]->dump(level);
                        std::cout << ") ";
                        children()[i + 1]->dump(level);
                        std::cout << ';' << std::endl;
                        i += 2;
                    } else {
                        std::cout << "else ";
                        children()[i]->dump(level);
                        std::cout << ';' << std::endl;
                        ++i;
                    }
                }
            }
            break;
        case StatementType::While:
            std::cout << "while (";
            children()[0]->dump(level);
            std::cout << ") ";
            children()[1]->dump(level);
            std::cout << std::endl;
            break;
        case StatementType::For:
            std::cout << "for (";
            children()[0]->dump(level);
            std::cout << "; ";
            children()[1]->dump(level);
            std::cout << "; ";
            children()[2]->dump(level);
            std::cout << ") ";
            children()[3]->dump(level);
            std::cout << std::endl;
            break;
        case StatementType::Switch:
            std::cout << "switch (";
            children()[0]->dump(level);
            std::cout << ") {" << std::endl;
            for (auto i = 1u; i < children().size(); ++i) {
                Tabify(level);
                if (i + 1 < children().size()) {
                    std::cout << "case ";
                    children()[i]->dump(level);
                    std::cout << ":" << std::endl;
                    Tabify(level + 1);
                    children()[i + 1]->dump(level + 1);
                    std::cout << ';' << std::endl;
                } else {
                    std::cout << "default:" << std::endl;
                    Tabify(level + 1);
                    children()[i]->dump(level + 1);
                    std::cout << ';' << std::endl;
                }
            }
            break;
        case StatementType::Break:
            std::cout << "break";
            break;
        case StatementType::Continue:
            std::cout << "continue";
            break;
        case StatementType::Return:
            std::cout << "return ";
            children()[0]->dump(level);
            break;
        case StatementType::Emit:
            std::cout << "emit ";
            children()[0]->dump(level);
            break;
        default:
            std::cout << "UNKNOWN STATEMENT" << std::endl;
            break;
        }
        break;
    case TokenType::TemplateType:
        std::cout << _token->literal() << '<';
        for (bool passed = false; const auto &child : children()) {
            if (passed)
                std::cout << ", ";
            else
                passed = true;
            child->dump(level);
        }
        std::cout << '>';
        break;
    case TokenType::Operator:
        if (!firstOperation)
            std::cout << '(';
        if (IsUnary(operatorType())) {
            if (operatorType() == OperatorType::IncrementSuffix || operatorType() == OperatorType::DecrementSuffix) {
                children()[0]->dump(level, false);
                std::cout << _token->literal();
            } else {
                std::cout << _token->literal();
                children()[0]->dump(level, false);
            }
        } else if (IsBinary(operatorType())) {
                children()[0]->dump(level, false);
                if (operatorType() == OperatorType::Dot)
                    std::cout << _token->literal();
                else
                    std::cout << " " << _token->literal() << " ";
                children()[1]->dump(level, false);
        } else if (IsTerciary(operatorType())) {
                children()[0]->dump(level, false);
                std::cout << " " << _token->literal() << " ";
                children()[1]->dump(level, false);
                std::cout << " : ";
                children()[2]->dump(level, false);
        } else if (operatorType() == OperatorType::Call) {
            children()[0]->dump(level, false);
            std::cout << '(';
            if (children()[1])
                children()[1]->dump(level, false);
            std::cout << ')';
        } else
            std::cout << "UNKNOWN OPERATOR";
        if (!firstOperation)
            std::cout << ')';
        break;
    case TokenType::Constant:
        std::cout << _token->literal();
        break;
    default:
        std::cout << "UNKNOWN TOKEN" << std::endl;
        break;
    }
}


