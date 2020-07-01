project(KubeInterpreterBenchmarks)

get_filename_component(KubeInterpreterBenchmarksDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(KubeInterpreterBenchmarksSources
    ${KubeInterpreterBenchmarksDir}/Main.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${KubeInterpreterBenchmarksSources})

target_link_libraries(${CMAKE_PROJECT_NAME}
PUBLIC
    KubeInterpreter
    benchmark::benchmark
)