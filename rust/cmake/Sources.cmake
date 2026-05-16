set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

add_library(compiler)

file(GLOB_RECURSE LEXER_HEADERS ${SRC_DIR}/**.hpp)
file(GLOB_RECURSE LEXER_SOURCES ${SRC_DIR}/**.cpp)
target_sources(compiler PRIVATE
        ${LEXER_SOURCES}
        ${LEXER_HEADERS}
)

target_include_directories(compiler PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include/compiler>
)

target_compile_definitions(compiler PUBLIC
        PROJECT_ROOT_LEN=${SOURCE_PATH_SIZE}
)

add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE compiler)