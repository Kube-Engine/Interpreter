project(KubeInterpreterTests)

get_filename_component(KubeInterpreterTestsDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(KubeInterpreterTestsSources
    ${KubeInterpreterTestsDir}/tests_Object.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${KubeInterpreterTestsSources})

add_test(NAME ${CMAKE_PROJECT_NAME} COMMAND ${CMAKE_PROJECT_NAME})

target_link_libraries(${CMAKE_PROJECT_NAME}
PUBLIC
    KubeInterpreter
    GTest::GTest GTest::Main
)