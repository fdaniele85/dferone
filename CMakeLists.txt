cmake_minimum_required(VERSION 3.14)
project(dferone VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# Header-only library target
# ============================================================================
add_library(dferone INTERFACE)

# Alias per uso consistente (importante per FetchContent)
add_library(dferone::dferone ALIAS dferone)

# Include directories
target_include_directories(dferone INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# C++20 requirement
target_compile_features(dferone INTERFACE cxx_std_20)

# Opzionale: thread safety
option(DFERONE_THREAD_SAFE "Enable thread-safe features" OFF)
if(DFERONE_THREAD_SAFE)
    target_compile_definitions(dferone INTERFACE DFERONE_THREAD_SAFE)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(dferone INTERFACE OpenMP::OpenMP_CXX)
    endif()
endif()

# ============================================================================
# Tests (solo se è il progetto principale)
# ============================================================================
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(DFERONE_BUILD_TESTS "Build tests" ON)

    if(DFERONE_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests)
    endif()

    # Documentation
    option(DFERONE_BUILD_DOCS "Build documentation" ON)
    if(DFERONE_BUILD_DOCS)
        find_package(Doxygen)
        if(DOXYGEN_FOUND)
            set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
            set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
            configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

            add_custom_target(dferone_docs
                    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    COMMENT "Generating dferone documentation"
                    VERBATIM
            )
        endif()
    endif()
endif()