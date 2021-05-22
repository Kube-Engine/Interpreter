/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Interpreter
 */

#include <fstream>
#include <filesystem>
#include <iostream>

#include <Kube/Flow/Scheduler.hpp>

#include "Interpreter.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

using namespace kF;

namespace kF::Lang
{
    /** @brief Lexer work functor */
    struct alignas_cacheline LexerWork
    {
        /** @brief Construct the lexer worker instance */
        LexerWork(Core::TinyString &&context_, std::unique_ptr<std::ifstream> istream_, const FileIndex file_)
            : context(std::move(context_)), istream(std::move(istream_)), file(file_) {}

        /** @brief Start lexer */
        void operator()(void)
        {
            try {
                stack = Lexer().run(file, *istream, context.toStdView());
            } catch (const std::exception &e) {
                crash = true;
                error = e.what();
            }
        }

        Core::TinyString context;
        Core::TinyString error;
        TokenStack stack;
        std::unique_ptr<std::ifstream> istream;
        FileIndex file;
        bool crash = false;
    };

    static_assert_fit_cacheline(LexerWork);

    /** @brief Parser work functor */
    struct alignas_cacheline ParserWork
    {
        /** @brief Construct the parser worker instance */
        ParserWork(Core::TinyString &&context_, const TokenStack *stack_, const FileIndex file_)
            : context(std::move(context_)), stack(std::move(stack_)), file(file_) {}

        /** @brief Start parser */
        void operator()(void)
        {
            try {
                Parser parser;
                node = parser.run(file, stack, context.toStdView());
                imports = std::move(parser.imports());
            } catch (const std::exception &e) {
                crash = true;
                error = e.what();
            }
        }

        Core::TinyString context;
        Core::TinyVector<Core::TinyString> imports;
        Core::FlatString error;
        const TokenStack *stack;
        AST::Ptr node;
        FileIndex file;
        bool crash = false;
    };

    static_assert_fit_cacheline(ParserWork);
}

Lang::Interpreter::Interpreter(Flow::Scheduler * const scheduler)
    : _scheduler(scheduler)
{
}

void Lang::Interpreter::run(const std::string_view &path)
{
    preprocessFile(path);
    while (constructGraph()) {
        _scheduler->schedule(_graph);
        _graph.wait();
        _scheduler->processNotifications();
    }
}

void Lang::Interpreter::preprocessFile(const std::string_view &path)
{
    const auto fileIndex = _directoryManager.discoverFile(path);
    std::string sPath(path);
    auto istream = std::make_unique<std::ifstream>(sPath);

    if (!*istream)
        throw std::logic_error("Lang::Interpreter::preprocessFile: Cannot load file '" + sPath + '\'');

    auto &p = _toLexer.push();
    auto lexerWork = new LexerWork(sPath, std::move(istream), fileIndex);

    // Lexer work node
    p.first.prepare<[](LexerWork *ptr) { delete ptr; }>(lexerWork);

    // Lexer notify node
    p.second.prepare([this, lexerWork] {
        if (lexerWork->crash) [[unlikely]]
            throw std::logic_error(lexerWork->error.c_str());

        // On file lexed success
        auto &fileStack = _directoryManager.fileStack(lexerWork->file) = std::move(lexerWork->stack);
        auto &p = _toParser.push();
        auto parserWork = new ParserWork(std::move(lexerWork->context), &fileStack, lexerWork->file);

        // Parser work node
        p.first.prepare<[](ParserWork *ptr) { delete ptr; }>(parserWork);

        // Parser notify node
        p.second.prepare([this, parserWork] {
            if (parserWork->crash) [[unlikely]]
                throw std::logic_error(parserWork->error.c_str());

            // On file parsed success
            auto &node = _directoryManager.fileNode(parserWork->file) = std::move(parserWork->node);

            // Add imports to directory manager
            for (const auto &import : parserWork->imports) {
                _directoryManager.discoverDirectory(import.toStdView());
            }

            // For each class within the file, check if it should be interpreted (using import scope)
            // node->traverse(
            //     [this, parserWork, fileDirectory = _directoryManager.fileDirectory(parserWork->file)](const AST &node) {
            //         if (node.type() != TokenType::Class) [[likely]]
            //             return false;

            //         const auto &rootFiles = _directoryManager.directoryFiles(fileDirectory);
            //         for (const auto file : rootFiles) {
            //             if (_directoryManager.fileName(file) == node.literal()) [[unlikely]] {
            //                 if (_directoryManager.fileStack(file).empty())
            //                     preprocessFile()
            //             }
            //         }

            //         return true;
            //     }
            // );

            node->dump();
        });
    });
}

bool Lang::Interpreter::constructGraph(void)
{
    _graph.clear();

    // Check if the graph has no further steps to perform
    if (_toLexer.empty() && _toParser.empty()) [[unlikely]]
        return false;

    // Add lexer tasks to the graph
    for (auto &p : _toLexer)
        _graph.emplace(std::move(p.first), std::move(p.second));
    _toLexer.clear();

    // Add parser tasks to the graph
    for (auto &p : _toParser)
        _graph.emplace(std::move(p.first), std::move(p.second));
    _toParser.clear();

    return true;
}