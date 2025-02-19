cmake_minimum_required(VERSION 3.14)
project(dferone)

cmake_policy(SET CMP0135 NEW)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # We're in the root, define additional targets for developers.
    #option(MY_PROJECT_BUILD_EXAMPLES   "whether or not examples should be built" ON)
    option(DFERONE_BUILD_TESTS "whether or not tests should be built" ON)

    #if(MY_PROJECT_BUILD_EXAMPLES)
    #    add_subdirectory(examples)
    #endif()
    if (DFERONE_BUILD_TESTS)
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
                #GRASP_test.cpp
        )

        if (OpenMP_CXX_FOUND)
            target_link_libraries(hello_test PUBLIC GTest::gtest_main OpenMP::OpenMP_CXX)
        else ()
            target_link_libraries(hello_test GTest::gtest_main)
        endif ()


        include(GoogleTest)
        gtest_discover_tests(hello_test)
    endif ()

    set(package_files containers/ console.h CMakeLists.txt LICENSE Doxyfile README.md Counter.cpp Counter.h random.h ranges.h streams.h algorithms/ vrp/AbstractInstance.hpp vrp/Instance.hpp vrp/SubInstance.hpp)
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-src.zip
            COMMAND ${CMAKE_COMMAND} -E tar c ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-src.zip --format=zip -- ${package_files}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${package_files})
    add_custom_target(${PROJECT_NAME}_package DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-src.zip)
endif ()

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
    add_custom_target(doc ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)


add_library(dferone Counter.cpp)