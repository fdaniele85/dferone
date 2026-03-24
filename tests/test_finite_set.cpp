#include <doctest/doctest.h>

#include <dferone/containers/FiniteSet.h>
#include <random>
#include <vector>

TEST_CASE("FiniteSet add/remove/contains basic invariants") {
    using dferone::containers::FiniteSet;

    FiniteSet<int> fs(10);
    CHECK(fs.size() == 0);
    CHECK(fs.empty());

    fs.add(3);
    fs.add(5);
    fs.add(5);

    CHECK(fs.size() == 2);
    CHECK(fs.contains(3));
    CHECK(fs.contains(5));
    CHECK_FALSE(fs.contains(4));

    fs.remove(3);
    CHECK_FALSE(fs.contains(3));
    CHECK(fs.contains(5));
    CHECK(fs.size() == 1);
}

TEST_CASE("FiniteSet contains out-of-range is safe and false") {
    using dferone::containers::FiniteSet;

    FiniteSet<int> fs(5);

    // These should be safe and return false.
    CHECK_FALSE(fs.contains(5));
    CHECK_FALSE(fs.contains(100));
}

TEST_CASE("FiniteSet supports shifted universe with first_element") {
    using dferone::containers::FiniteSet;

    FiniteSet<int> fs(5, 0, 5); // universe [5, 10)

    CHECK(fs.capacity() == 5);
    CHECK(fs.first_element() == 5);
    CHECK_FALSE(fs.contains(4));
    CHECK_FALSE(fs.contains(10));

    fs.add(5);
    fs.add(9);
    fs.add(9);

    CHECK(fs.size() == 2);
    CHECK(fs.contains(5));
    CHECK(fs.contains(9));

    fs.remove(5);
    CHECK_FALSE(fs.contains(5));
    CHECK(fs.contains(9));
}

TEST_CASE("FiniteSet shifted constructor pre-fills from first_element") {
    using dferone::containers::FiniteSet;

    FiniteSet<int> fs(5, 3, 5); // [5, 10), initially {5,6,7}

    CHECK(fs.size() == 3);
    CHECK(fs.contains(5));
    CHECK(fs.contains(6));
    CHECK(fs.contains(7));
    CHECK_FALSE(fs.contains(8));
}

TEST_CASE("FiniteSet behaves like a reference boolean set under random operations") {
    using dferone::containers::FiniteSet;

    constexpr int CAP = 50;
    FiniteSet<int> fs(CAP);
    std::vector<bool> ref(CAP, false);

    std::mt19937 rng(123);
    std::uniform_int_distribution<int> el_dist(0, CAP - 1);
    std::bernoulli_distribution op(0.5);

    for (int step = 0; step < 5000; ++step) {
        int el = el_dist(rng);
        if (op(rng)) {
            fs.add(el);
            ref[el] = true;
        } else {
            fs.remove(el);
            ref[el] = false;
        }

        // Check membership consistency for a few sampled elements
        for (int k = 0; k < 5; ++k) {
            int q = el_dist(rng);
            CHECK(fs.contains(q) == ref[q]);
        }
    }
}
