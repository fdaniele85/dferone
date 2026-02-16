#include <doctest/doctest.h>

#include <dferone/containers/Matrix.h>

TEST_CASE("Matrix basic set/get and const access") {
    using dferone::containers::Matrix;

    Matrix<int> m(3, 4, 0);
    m(2, 3) = 42;

    CHECK(m(2, 3) == 42);

    // Force instantiation of const operator()(row,col)
    const Matrix<int>& cm = m;
    CHECK(cm(2, 3) == 42);
}
