/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Abstract Syntax Tree
 */

#include <iostream>

#include "AST.hpp"

using namespace kF;

void Lang::AST::dump(const std::size_t level) const noexcept
{
    constexpr auto Tabify = [](const std::size_t level) {
        std::cout << std::string(level * 2, ' ');
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
        Tabify(level);
        std::cout << "property ";
        children()[0]->dump(level);
        std::cout << ": ";
        children()[1]->dump(level);
        std::cout << std::endl;
        break;
    case TokenType::Signal:
        std::cout << "signal " << _token->literal();
        children()[0]->dump(level);
        std::cout << ";" << std::endl;
        break;
    case TokenType::Function:
        std::cout << "function " << _token->literal();
        children()[0]->dump(level);
        std::cout << " ";
        children()[1]->dump(level);
        std::cout << std::endl;
        break;
    case TokenType::Event:
        std::cout << "on ";
        children()[0]->dump(level);
        std::cout << ": ";
        children()[1]->dump(level);
        std::cout << std::endl;
        break;
    case TokenType::Assignment:
        std::cout << _token->literal() << ": ";
        children()[0]->dump(level);
        std::cout << std::endl;
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
        else if (children().size() == 1u)
            children()[0]->dump(level);
        else {
            std::cout << '{' << std::endl;
            for (const auto &child : children()) {
                Tabify(level + 1);
                child->dump(level + 1);
                std::cout << std::endl;
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
        std::cout << ';' << std::endl;
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
                        std::cout << std::endl;
                        i += 2;
                    } else {
                        std::cout << "else ";
                        children()[i]->dump(level);
                        std::cout << std::endl;
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
                    std::cout << std::endl;
                } else {
                    std::cout << "default:" << std::endl;
                    Tabify(level + 1);
                    children()[i]->dump(level + 1);
                    std::cout << std::endl;
                }
            }
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
    case TokenType::UnaryOperator:
        std::cout << "UnaryOperator" << std::endl;
        break;
    case TokenType::BinaryOperator:
        std::cout << "BinaryOperator" << std::endl;
        break;
    case TokenType::Call:
        std::cout << "Call" << std::endl;
        break;
    case TokenType::Emit:
        std::cout << "Emit" << std::endl;
        break;
    case TokenType::Constant:
        std::cout << "Constant" << std::endl;
        break;
    default:
        std::cout << "UNKNOWN TOKEN" << std::endl;
        break;
    }
}


