set(TEST_SRC ${CMAKE_SOURCE_DIR}/tests)
enable_testing()

function(lexer_add_test)
    set(TEST_TARGET ${ARGV0})
    string(REPLACE "/" "-" TEST_TARGET ${TEST_TARGET})
    set(TEST_DIR ${TEST_SRC}/${ARGV0})

    file(GLOB_RECURSE TEST_HEADERS ${TEST_DIR}/*.hpp)
    file(GLOB_RECURSE TEST_SOURCES ${TEST_DIR}/*.cpp)

    add_executable(${TEST_TARGET} ${TEST_SOURCES} ${TEST_HEADERS})
    target_link_libraries(${TEST_TARGET} GTest::gtest_main lexer)

    add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

    target_include_directories(${TEST_TARGET} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:include/lexer>
    )
endfunction()