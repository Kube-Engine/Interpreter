project(KubeInterpreter)

get_filename_component(KubeInterpreterDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(KubeInterpreterSources
    ${KubeInterpreterDir}/Make.hpp
    ${KubeInterpreterDir}/Reflection.hpp
    ${KubeInterpreterDir}/Reflection.cpp
    ${KubeInterpreterDir}/Object.hpp
    ${KubeInterpreterDir}/Object.ipp
    ${KubeInterpreterDir}/Object.cpp
)

add_library(${PROJECT_NAME} ${KubeInterpreterSources})

target_link_libraries(${PROJECT_NAME}
PUBLIC
    KubeCore
    KubeMeta
)

if(${KF_TESTS})
    include(${KubeInterpreterDir}/Tests/InterpreterTests.cmake)
endif()

if(${KF_BENCHMARKS})
    include(${KubeInterpreterDir}/Benchmarks/InterpreterBenchmarks.cmake)
endif()