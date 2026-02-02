# dferone

[![CI](https://github.com/fdaniele85/dferone/actions/workflows/ci.yml/badge.svg)](https://github.com/fdaniele85/dferone/actions/workflows/ci.yml)
[![Documentation](https://github.com/fdaniele85/dferone/actions/workflows/docs.yml/badge.svg)](https://fdaniele85.github.io/dferone/)
[![Release](https://img.shields.io/github/v/release/fdaniele85/dferone)](https://github.com/fdaniele85/dferone/releases)

C++20 header-only library for optimization algorithms and data structures.

📚 [Documentation](https://fdaniele85.github.io/dferone/) | 🚀 [Releases](https://github.com/fdaniele85/dferone/releases)

## Installation

### Using FetchContent (CMake 3.14+)
```cmake
include(FetchContent)

FetchContent_Declare(
    dferone
    GIT_REPOSITORY https://github.com/fdaniele85/dferone.git
    GIT_TAG v1.0.0  # or main for latest
)

FetchContent_MakeAvailable(dferone)

target_link_libraries(your_target PRIVATE dferone::dferone)
```

## Features

- Header-only library
- C++20 required
- Optional thread-safety with OpenMP
- Comprehensive test suite
- Full API documentation

## Building Tests
```bash
cmake -B build -DDFERONE_BUILD_TESTS=ON
cmake --build build
cd build && ctest
```

## Building Documentation
```bash
cmake -B build -DDFERONE_BUILD_DOCS=ON
cmake --build build --target dferone_docs
# Open build/html/index.html
```