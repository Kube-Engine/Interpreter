project(KubeInterpreter)

get_filename_component(KubeInterpreterDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(KubeInterpreterBinary KubeInterpreterApp)

set(KubeInterpreterSources
    ${KubeInterpreterDir}/Base.hpp
    ${KubeInterpreterDir}/Lexer.hpp
    ${KubeInterpreterDir}/Lexer.ipp
    ${KubeInterpreterDir}/Lexer.cpp
    ${KubeInterpreterDir}/Parser.hpp
    ${KubeInterpreterDir}/Parser.ipp
    ${KubeInterpreterDir}/Parser.cpp
    ${KubeInterpreterDir}/TokenStack.hpp
    ${KubeInterpreterDir}/TokenStack.ipp
    ${KubeInterpreterDir}/DirectoryManager.hpp
    ${KubeInterpreterDir}/DirectoryManager.cpp
    ${KubeInterpreterDir}/AST.hpp
    ${KubeInterpreterDir}/AST.cpp
    ${KubeInterpreterDir}/Interpreter.hpp
    ${KubeInterpreterDir}/Interpreter.cpp
)

add_library(${PROJECT_NAME} ${KubeInterpreterSources})

target_link_libraries(${PROJECT_NAME}
PUBLIC
    KubeObject
    KubeFlow
    AtomicWait
)


# Add the interpreter executable
add_executable(${KubeInterpreterBinary} ${KubeInterpreterDir}/Main.cpp)

target_link_libraries(${KubeInterpreterBinary}
PUBLIC
    KubeInterpreter
)

if(${KF_TESTS})
    include(${KubeInterpreterDir}/Tests/InterpreterTests.cmake)
endif()

if(${KF_BENCHMARKS})
    include(${KubeInterpreterDir}/Benchmarks/InterpreterBenchmarks.cmake)
endif()