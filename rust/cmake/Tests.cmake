function(add_unit_test target_name)
    add_executable(${target_name} ${ARGN})

    target_link_libraries(${target_name} PRIVATE compiler GTest::gtest GTest::gtest_main)
    target_compile_definitions(${target_name} PRIVATE RUST_TESTING)

    add_test(NAME ${target_name} COMMAND compiler)

    set_tests_properties(${target_name} PROPERTIES WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endfunction()

set(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)

add_unit_test(lexer-unit ${TEST_DIR}/lexer.cpp)
add_unit_test(parser-unit ${TEST_DIR}/parser.cpp)
