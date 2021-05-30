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

void Lang::Interpreter::preprocessFile(const std::string_view &path, const FileIndex fileIndex)
{
    std::string sPath(path);
    auto istream = std::make_unique<std::ifstream>(sPath);

    if (!*istream)
        throw std::logic_error("Lang::Interpreter::preprocessFile: Cannot load file '" + sPath + '\'');

    // Register the file as being lexed this run
    _lexingList.push(fileIndex);

    auto &p = _toLexer.push();
    auto lexerWork = new LexerWork(sPath, std::move(istream), fileIndex);

    // Lexer work node
    p.work.prepare<[](LexerWork *ptr) { delete ptr; }>(lexerWork);

    // Lexer notify node
    p.notify.prepare([this, lexerWork] {
        // Remove the file from the lexing list
        _lexingList.erase(_lexingList.find(lexerWork->file));

        if (lexerWork->crash) [[unlikely]]
            throw std::logic_error(lexerWork->error.c_str());

        // On file lexed success
        auto &fileStack = _directoryManager.fileStack(lexerWork->file) = std::move(lexerWork->stack);
        auto &p = _toParser.push();
        auto parserWork = new ParserWork(std::move(lexerWork->context), &fileStack, lexerWork->file);;

        // Parser work node
        p.work.prepare<[](ParserWork *ptr) { delete ptr; }>(parserWork);

        // Parser notify node
        p.notify.prepare([this, parserWork] {
            if (parserWork->crash) [[unlikely]]
                throw std::logic_error(parserWork->error.c_str());


            // Add imports to directory manager
            Core::TinySmallVector<DirectoryIndex, Core::CacheLineQuarterSize / sizeof(DirectoryIndex)> importIndexes;
            importIndexes.reserve(parserWork->imports.size());
            for (const auto &import : parserWork->imports) {
                importIndexes.push(_directoryManager.discoverDirectory(import.toStdView()));
            }

            // Add the parsed node to the manager list
            auto &node = _directoryManager.fileNode(parserWork->file);
            node = std::move(parserWork->node);

            // For each class within the file, check if it should be interpreted (using import scope)
            node->traverse(
                [this, &importIndexes, fileDirectory = _directoryManager.fileDirectory(parserWork->file)](const AST &node) {
                    if (node.type() != TokenType::Class) [[likely]]
                        return false;

                    const auto className = node.literal();
                    const auto directoryClassSearch = [this, &className](const auto dirIndex) {
                        for (const auto file : _directoryManager.directoryFiles(dirIndex)) {
                            if (_directoryManager.fileName(file) == className) [[unlikely]] {
                                // Don't process the file if it already is or if it's planned for this run
                                if (_directoryManager.fileStack(file).empty() && _lexingList.find(file) == _lexingList.end()) {
                                    preprocessFile(_directoryManager.filePath(file).toStdView(), file);
                                    return true;
                                }
                            }
                        }
                        return false;
                    };

                    // Search class in current directory
                    if (!directoryClassSearch(fileDirectory)) {
                        // Search class in all imported directories
                        for (const auto dirIndex : importIndexes) {
                            if (directoryClassSearch(dirIndex))
                                break;
                        }
                    }
                    return true;
                }
            );

            std::cout << "'" << parserWork->context.c_str() << "':" << std::endl;
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
        _graph.emplace(std::move(p.work), std::move(p.notify));
    _toLexer.clear();

    // Add parser tasks to the graph
    for (auto &p : _toParser)
        _graph.emplace(std::move(p.work), std::move(p.notify));
    _toParser.clear();

    return true;
}