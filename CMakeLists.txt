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
add_compile_options(-fconcepts-diagnostics-depth=3)

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

option(BUILD_DOC "Build documentation" ON)

# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target( doc ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)
