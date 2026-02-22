set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
include(FetchContent)

# doctest
FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

add_executable(dferone_tests
        doctest_main.cpp
        test_matrix.cpp
        test_symmetric_matrix.cpp
        test_finite_set.cpp
        test_welford.cpp
        test_bestset_sortedvector.cpp
        test_tolerance_compile.cpp
)

target_link_libraries(dferone_tests PRIVATE
        dferone::dferone
        doctest::doctest
)

# Warning utili solo per i test (facoltativo)
target_compile_options(dferone_tests PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

include(CTest)
add_test(NAME dferone_tests COMMAND dferone_tests)
