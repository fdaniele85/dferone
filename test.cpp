#include <gtest/gtest.h>

#include "containers/BestSet.h"
#include "containers/FiniteSet.h"
#include "streams.h"
#include "random.h"

namespace {
    using namespace dferone::containers;
    using namespace dferone::streams;
    using namespace dferone::random;

    TEST(Containers, best_set) {
        BestSet<int> bs(5);
        ASSERT_EQ(bs.size(), 0);
        bs.add(10);
        ASSERT_GT(bs.size(), 0);
        ASSERT_EQ(bs.top(), 10);
        bs.add(15);
        ASSERT_EQ(bs.top(), 15);
        ASSERT_EQ(bs.size(), 2);
        bs.add(1);
        bs.add(2);
        ASSERT_TRUE(bs.add(3));
        ASSERT_FALSE(bs.add(1));
        ASSERT_EQ(bs.top(), 15);
        bs.pop();
        ASSERT_EQ(bs.size(), 4);
        ASSERT_EQ(bs.top(), 10);
    }

    TEST(Containers, finite_set) {
        FiniteSet<uint> fs(5);
        ASSERT_EQ(fs.size(), 0);
        fs.add(10);
        ASSERT_EQ(fs.size(), 0);
        fs.add(4);
        ASSERT_TRUE(fs.contains(4));
        ASSERT_EQ(fs.count(4), 1);
        ASSERT_FALSE(fs.contains(1));
        ASSERT_EQ(fs.count(1), 0);
        ASSERT_EQ(fs.size(), 1);
        fs.add(1);
        fs.add(2);
        fs.add(3);
        fs.add(1);
        ASSERT_EQ(fs.size(), 4);
        fs.remove(4);
        ASSERT_FALSE(fs.contains(4));
        ASSERT_EQ(fs.size(), 3);
    }

    TEST(streams, j_and_p) {
        std::vector<int> v {1, 2, 3};
        std::ostringstream ss;
        join_and_print(ss, v);
        ASSERT_EQ(ss.str(), "1, 2, 3");
    }



}