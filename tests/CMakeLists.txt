cmake_minimum_required(VERSION 3.14)

# Fetch Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Enable testing
include(GoogleTest)

# Test executables
add_executable(test_audio
    test_audio.cpp
    test_main.cpp
)

target_link_libraries(test_audio PRIVATE
    bsc_core
    GTest::gtest
    GTest::gtest_main
)

target_include_directories(test_audio PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/detect
)

# Discover tests
gtest_discover_tests(test_audio)

# Add custom target to run tests
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_audio
    COMMENT "Running tests..."
)

message(STATUS "Tests configured - run 'make test' or 'make check'")
