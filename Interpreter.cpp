/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Interpreter
 */

#include <fstream>

#include <Kube/Flow/Scheduler.hpp>

#include "Interpreter.hpp"

using namespace kF;

void Lang::Interpreter::process(const std::string_view &path)
{
    std::string sPath(path);
    std::ifstream ss(sPath);

    if (!ss)
        throw std::logic_error("Lang::Interpreter::process: Cannot load file '" + sPath + '\'');

}

void Lang::Interpreter::onFileLexed(const FileIndex file, TokenStack &&stack)
{

}
