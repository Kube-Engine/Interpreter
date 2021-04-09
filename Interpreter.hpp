/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Interpreter
 */

#pragma once

#include <Kube/Core/String.hpp>
#include <Kube/Core/SmallString.hpp>
#include <Kube/Flow/Graph.hpp>

#include "DirectoryManager.hpp"

namespace kF::Lang
{
    class Interpreter;
}

// Forward declaration of the scheduler
namespace kF::Flow { class Scheduler; }

class kF::Lang::Interpreter
{
public:
    /** @brief Constructor */
    Interpreter(Flow::Scheduler * const scheduler);

    /** @brief Move constructor */
    Interpreter(Interpreter &&other) = default;

    /** @brief Destructor */
    ~Interpreter(void) = default;

    /** @brief Run the interpreter in blocking mdoe */
    void run(const std::string_view &path);

private:
    DirectoryManager _directoryManager {};
    Flow::Scheduler *_scheduler { nullptr };
    Flow::Graph _graph {};
    Core::TinyVector<std::pair<Flow::StaticFunc, Flow::NotifyFunc>> _toLexer {};
    Core::TinyVector<std::pair<Flow::StaticFunc, Flow::NotifyFunc>> _toParser {};

    /** @brief Process a file */
    void preprocessFile(const std::string_view &path);

    /** @brief Construct the next graph */
    [[nodiscard]] bool constructGraph(void);
};
