#include <gtest/gtest.h>

#include "containers/BestSet.h"

namespace {
    using namespace dferone::containers;

// Demonstrate some basic assertions.
    TEST(HelloTest, best_set) {
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

}