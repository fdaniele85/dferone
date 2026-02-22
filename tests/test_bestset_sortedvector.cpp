#include <doctest/doctest.h>

#include <dferone/containers/BestSet.h>
#include <dferone/containers/SortedVector.h>

TEST_CASE("SortedVector keeps elements ordered") {
    dferone::containers::SortedVector<int> sv;
    sv.add(5);
    sv.add(1);
    sv.add(3);
    sv.add(3);

    CHECK(sv.size() == 4);
    CHECK(sv[0] == 1);
    CHECK(sv[1] == 3);
    CHECK(sv[2] == 3);
    CHECK(sv[3] == 5);
}

TEST_CASE("BestSet keeps top-k by comparator") {
    // Default comparator is std::greater => best elements are larger
    dferone::containers::BestSet<int> bs(3);

    bs.add(5);
    bs.add(1);
    bs.add(3);
    bs.add(10);
    bs.add(2);

    CHECK(bs.size() == 3);
    CHECK(bs.top() == 10);

    // Should contain {10,5,3} in that order
    CHECK(bs[0] == 10);
    CHECK(bs[1] == 5);
    CHECK(bs[2] == 3);
}
