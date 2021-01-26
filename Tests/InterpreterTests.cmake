project(KubeInterpreterTests)

get_filename_component(KubeInterpreterTestsDir ${CMAKE_CURRENT_LIST_FILE} PATH)

set(KubeInterpreterTestsSources
    ${KubeInterpreterTestsDir}/tests_Interpreter.cpp
    ${KubeInterpreterTestsDir}/tests_TokenStack.cpp
    ${KubeInterpreterTestsDir}/tests_Lexer.cpp
)

add_executable(${CMAKE_PROJECT_NAME} ${KubeInterpreterTestsSources})

add_test(NAME ${CMAKE_PROJECT_NAME} COMMAND ${CMAKE_PROJECT_NAME})

target_link_libraries(${CMAKE_PROJECT_NAME}
PUBLIC
    KubeInterpreter
    GTest::GTest GTest::Main
)

if(KF_COVERAGE)
    target_compile_options(${PROJECT_NAME} PUBLIC --coverage)
    target_link_options(${PROJECT_NAME} PUBLIC --coverage)
endif()