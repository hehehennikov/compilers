include(FetchContent)

# gtest
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
        OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(googletest)