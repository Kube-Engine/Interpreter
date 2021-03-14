/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Interpreter
 */

#pragma once

#include <Kube/Core/String.hpp>
#include <Kube/Core/SmallString.hpp>
#include <Kube/Flow/Graph.hpp>

#include "TokenStack.hpp"

namespace kF::Lang
{
    class Interpreter;
}

class kF::Lang::Interpreter
{
public:
    /** @brief File descriptor */
    struct alignas_cacheline File
    {
        Core::TinySmallString name;
        Core::TinyString path;
        TokenStack stack;
        // AST tree;
    };

    /** @brief Contains data about the current parsing state */
    struct ParseContext
    {
    };

    static_assert_fit_cacheline(File);

    /** @brief Process a file */
    void process(const std::string_view &path);

    /** @brief Process a stream */
    void process(std::istream &&istream);

private:
    Core::TinyVector<std::unique_ptr<File>> _files {};
    Core::TinyVector<FileIndex> _lexingFiles {};
    Core::TinyVector<FileIndex> _parsingFiles {};
    Core::TinyVector<FileIndex> _instantiatingFiles {};
    kF::Flow::Graph _graph {};

    void onFileLexed(const FileIndex file);
    void onFileParsed(const FileIndex file);
    void onFileInstantiated(const FileIndex file);
};

#include "Interpreter.ipp"