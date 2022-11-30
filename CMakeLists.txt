cmake_minimum_required(VERSION 3.14)
project(dferone)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(OpenMP)

enable_testing()

add_compile_definitions(DFERONE_THREAD_SAFE)

add_executable(
        hello_test
        test.cpp
        Counter.cpp
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(hello_test PUBLIC GTest::gtest_main OpenMP::OpenMP_CXX)
else()
    target_link_libraries(hello_test GTest::gtest_main)
endif()


include(GoogleTest)
gtest_discover_tests(hello_test)
