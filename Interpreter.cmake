project(KubeInterpreter)

get_filename_component(KubeInterpreterDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(KubeInterpreterSources
    ${KubeInterpreterDir}/Base.hpp
    ${KubeInterpreterDir}/Lexer.hpp
    ${KubeInterpreterDir}/Lexer.ipp
    ${KubeInterpreterDir}/Lexer.cpp
    ${KubeInterpreterDir}/TokenStack.hpp
    ${KubeInterpreterDir}/TokenStack.ipp
)

add_library(${PROJECT_NAME} ${KubeInterpreterSources})

target_link_libraries(${PROJECT_NAME}
PUBLIC
    KubeObject
)

if(${KF_TESTS})
    include(${KubeInterpreterDir}/Tests/InterpreterTests.cmake)
endif()

if(${KF_BENCHMARKS})
    include(${KubeInterpreterDir}/Benchmarks/InterpreterBenchmarks.cmake)
endif()