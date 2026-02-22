#include <doctest/doctest.h>

#include <dferone/welford.h>

#include <cmath>
#include <vector>

TEST_CASE("WelfordAlgorithm empty stats") {
    dferone::WelfordAlgorithm w;

    CHECK(w.get_mean() == doctest::Approx(0.0));

    // Expected behavior for empty stream: variance/stddev should be 0.
    // If you prefer NaN, change this test accordingly.
    CHECK(w.get_variance() == doctest::Approx(0.0));
    CHECK(w.get_std_dev() == doctest::Approx(0.0));
}

TEST_CASE("WelfordAlgorithm matches known mean/variance") {
    dferone::WelfordAlgorithm w;

    std::vector<double> xs{1.0, 2.0, 3.0, 4.0};
    for (double x : xs) w.addElement(x);

    // mean = 2.5
    CHECK(w.get_mean() == doctest::Approx(2.5));

    // population variance for {1,2,3,4} is 1.25
    CHECK(w.get_variance() == doctest::Approx(1.25));
    CHECK(w.get_std_dev() == doctest::Approx(std::sqrt(1.25)));
}
