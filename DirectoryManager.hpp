/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Directory manager
 */

#pragma once

#include <Kube/Core/Vector.hpp>
#include <Kube/Core/SmallVector.hpp>
#include <Kube/Core/String.hpp>

#include "TokenStack.hpp"
#include "AST.hpp"

namespace kF::Lang
{
    class DirectoryManager;
}

class kF::Lang::DirectoryManager
{
public:
    /** @brief Discover each file in a directory*/
    [[nodiscard]] DirectoryIndex discoverDirectory(const std::string_view &path, const bool acceptFilePath = false);

    /** @brief Register a file and discover its directory */
    [[nodiscard]] FileIndex discoverFile(const std::string_view &path);


    /** @brief Get a file's name */
    [[nodiscard]] const Core::TinyString &fileName(const FileIndex fileIndex) const noexcept { return _fileNames[fileIndex]; }

    /** @brief Get a file's directory index */
    [[nodiscard]] DirectoryIndex fileDirectory(const FileIndex fileIndex) const noexcept { return _fileDirectories[fileIndex]; }

    /** @brief Get a file's token stack */
    [[nodiscard]] TokenStack &fileStack(const FileIndex fileIndex) noexcept { return _fileStacks[fileIndex]; }
    [[nodiscard]] const TokenStack &fileStack(const FileIndex fileIndex) const noexcept { return _fileStacks[fileIndex]; }

    /** @brief Get a file's node */
    [[nodiscard]] AST::Ptr &fileNode(const FileIndex fileIndex) noexcept { return _fileNodes[fileIndex]; }
    [[nodiscard]] const AST::Ptr &fileNode(const FileIndex fileIndex) const noexcept { return _fileNodes[fileIndex]; }

    /** @brief Get the list of files in a directory */
    [[nodiscard]] const auto &directoryPath(const DirectoryIndex dirIndex) const noexcept { return _directoryPaths[dirIndex]; }

    /** @brief Get the list of files in a directory */
    [[nodiscard]] const auto &directoryFiles(const DirectoryIndex dirIndex) const noexcept { return _directoryFiles[dirIndex]; }

private:
    // Files
    Core::TinyVector<Core::TinyString> _fileNames;
    Core::TinyVector<DirectoryIndex> _fileDirectories;
    Core::TinyVector<TokenStack> _fileStacks;
    Core::TinyVector<AST::Ptr> _fileNodes;

    // Directories
    Core::TinyVector<Core::TinyString> _directoryPaths;
    Core::TinyVector<Core::TinySmallVector<FileIndex, (Core::CacheLineSize - Core::CacheLineQuarterSize) / sizeof(FileIndex)>> _directoryFiles;
};