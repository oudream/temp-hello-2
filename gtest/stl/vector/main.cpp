#include <gtest/gtest.h>

#include <vector>
#include <numeric>

int add(int a, int b) { return a + b; }

TEST(MathSuite, AddWorks) {
    EXPECT_EQ(add(1, 2), 3);
    EXPECT_NE(add(2, 2), 3);
}

TEST(AlgoSuite, AccumulateVector) {
    std::vector<int> v(5, 1);
    int sum = std::accumulate(v.begin(), v.end(), 0);
    EXPECT_EQ(sum, 5);
}
