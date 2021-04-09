/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Interpreter entry point
 */

#include <iostream>

#include <Kube/Flow/Scheduler.hpp>
#include <Kube/Interpreter/Interpreter.hpp>

static const char *Usage =
"Usage: ./KubeInterpreter [Flags] FilePath\n"
"\n"
"Arguments (flags must be before the file path):\n"
"  Flags:\n"
"    -h: Show this menu\n"
"  FilePath:\n"
"    The root .kl file\n";

int main(int ac, const char *av[])
{
    try {
        if (ac < 2)
            throw std::logic_error("main: No arguments");
        for (auto i = 1, max = ac - 1; i < ac; ++i) {
            auto arg = std::string_view(av[i]);
            if (arg == "-h") {
                std::cout << Usage << std::endl;
                return 0;
            } else if (i != max)
                throw std::logic_error("main: Unknown argument '" + std::string(arg) + '\'');
        }

        kF::Flow::Scheduler scheduler;
        kF::Lang::Interpreter interpreter(&scheduler);

        auto arg = std::string_view(av[ac - 1]);
        std::cout << "Interpreter now running over root file '" << arg << '\'' << std::endl;
        interpreter.run(arg);

    } catch (const std::exception &e) {
        std::cout << "A critical error occured:\n" << e.what() << std::endl;
        return 1;
    }
}