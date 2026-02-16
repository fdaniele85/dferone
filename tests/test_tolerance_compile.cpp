// This file is intentionally small and includes tolerance.h alone.
// It catches missing standard includes (e.g., <algorithm> for std::max).

#include <dferone/tolerance.h>

#include <doctest/doctest.h>

TEST_CASE("Tolerance basic comparisons") {
    dferone::Tolerance tol(1e-9);
    CHECK(tol.equal(1.0, 1.0 + 1e-12));
    CHECK(tol.less(1.0, 2.0));
    CHECK(tol.greater(2.0, 1.0));
}
