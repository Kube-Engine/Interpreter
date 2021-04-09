/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of DirectoryManager
 */

#include <gtest/gtest.h>

#include <Kube/Core/StringUtils.hpp>
#include <Kube/Interpreter/DirectoryManager.hpp>

using namespace kF;

TEST(DirectoryManager, Basics)
{
    Lang::DirectoryManager manager;

    auto dir = manager.discoverDirectory(".");
    std::cout << "Directory: " << manager.directoryPath(dir) << std::endl;
    for (auto file : manager.directoryFiles(dir)) {
        std::cout << manager.fileName(file) << std::endl;
    }
}
