set(SRC_DIR ${CMAKE_SOURCE_DIR}/src/lexer)

add_library(lexer)

file(GLOB_RECURSE LEXER_HEADERS ${SRC_DIR}/**.hpp)
file(GLOB_RECURSE LEXER_SOURCES ${SRC_DIR}/**.cpp)
target_sources(lexer PRIVATE
        ${LEXER_SOURCES}
        ${LEXER_HEADERS}
)

target_include_directories(lexer PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include/lexer>
)

target_compile_definitions(lexer PUBLIC
    PROJECT_ROOT_LEN=${SOURCE_PATH_SIZE}
)

add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE lexer)