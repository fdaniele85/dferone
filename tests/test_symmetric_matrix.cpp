#include <doctest/doctest.h>

#include <dferone/containers/SymmetricMatrix.h>

TEST_CASE("SymmetricMatrix aliases (i,j) and (j,i)") {
    using dferone::containers::SymmetricMatrix;

    SymmetricMatrix<int> a(5, 0);

    a(2, 3) = 7;
    CHECK(a(3, 2) == 7);

    a(4, 0) = 99;
    CHECK(a(0, 4) == 99);
}

TEST_CASE("SymmetricMatrix initializes full symmetric view") {
    using dferone::containers::SymmetricMatrix;

    SymmetricMatrix<int> a(4, 123);
    for (std::size_t i = 0; i < 4; ++i) {
        for (std::size_t j = 0; j < 4; ++j) {
            CHECK(a(i, j) == 123);
        }
    }
}
