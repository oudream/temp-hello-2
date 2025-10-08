#include <gtest/gtest.h>
#include <string>
#include <numeric>

int add(int a, int b) { return a + b; }

TEST(MathSuite, AddWorks) {
    EXPECT_EQ(add(1, 2), 3);
    EXPECT_NE(add(2, 2), 3);
}

TEST(StringSuite, SizeIsCorrect) {
    std::string s = "hello";
    EXPECT_EQ(s.size(), 5u);
}
