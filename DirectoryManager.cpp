/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Directory Manager
 */

#include <filesystem>

#include "DirectoryManager.hpp"

using namespace kF;

Lang::DirectoryIndex Lang::DirectoryManager::discoverDirectory(const std::string_view &path, const bool acceptFilePath)
{
    std::filesystem::path dirPath(std::filesystem::absolute(path));

    if (!std::filesystem::is_directory(dirPath)) {
        if (acceptFilePath)
            dirPath = dirPath.parent_path();
        else
            throw std::logic_error("Lang::DirectoryManager::discoverDirectory: Directory not found '" + std::string(path) + '\'');
    }

    std::string_view sDirPath = dirPath.c_str();

    // Try to find an already existing index
    for (DirectoryIndex index = 0; const auto &dir : _directoryPaths) {
        if (dir == sDirPath)
            return index;
        else
            ++index;
    }

    // Assign a new directory index
    const DirectoryIndex dirIndex = _directoryPaths.size();
    _directoryPaths.push(sDirPath);
    auto &files = _directoryFiles.push();
    for (const auto &entry : std::filesystem::directory_iterator(dirPath)) {
        if (!entry.is_regular_file())
            continue;
        auto filePath = entry.path();
        auto filename = filePath.filename();
        if (!filename.has_extension())
            continue;
        else if (auto ext = filename.extension().string(); ext.size() != 3 || std::toupper(ext[1]) != 'K' || std::toupper(ext[2]) != 'L')
            continue;
        filename.replace_extension();
        files.push(_filePaths.size());
        _filePaths.push(filePath.c_str());
        _fileNames.push(filename.c_str());
        _fileDirectories.push(dirIndex);
        _fileStacks.push();
        _fileNodes.push();
    }
    return dirIndex;
}

Lang::FileIndex Lang::DirectoryManager::discoverFile(const std::string_view &path)
{
    const auto dirIndex = discoverDirectory(path, true);
    const auto filename = std::filesystem::path(path).filename().replace_extension().string();
    for (const auto fileIndex : _directoryFiles[dirIndex]) {
        if (_fileNames[fileIndex] == filename)
            return fileIndex;
    }
    throw std::runtime_error("Lang::DirectoryManager: An error occured while discovering file '" + std::string(path) + '\'');
}
