#include <gtest/gtest.h>
#include "multiply.h"

TEST(MultiplyTest, PositiveNumbers) {
    EXPECT_EQ(multiply(2, 3), 6);
}

TEST(MultiplyTest, NegativeNumbers) {
    EXPECT_EQ(multiply(-4, -5), 20);
}

TEST(MultiplyTest, MixedSigns) {
    EXPECT_EQ(multiply(-3, 7), -21);
}

TEST(MultiplyTest, Zero) {
    EXPECT_EQ(multiply(0, 100), 0);
}
