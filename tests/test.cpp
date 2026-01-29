#include <gtest/gtest.h>

#include "../Counter.h"
#include "../console.h"
#include "../containers/BestSet.h"
#include "../containers/FiniteSet.h"
#include "../containers/Matrix.h"
#include "../containers/SoterdVector.h"
#include "../containers/SymmetricMatrix.h"
#include "../containers/containers.h"
#include "../random.h"
#include "../utilities.h"
#include "../welford.h"
#include <iterator>

namespace {
    using namespace dferone::containers;
    using namespace dferone::random;
    using namespace dferone::console;
    using namespace dferone::counters;

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

        ASSERT_TRUE(contains(bs, 1));
    }

    TEST(Containers, sorted_set) {
        SortedVector<int> bs;
        bs.add(10);
        ASSERT_EQ(bs.size(), 1);
        ASSERT_EQ(bs.front(), 10);
        bs.add(15);
        ASSERT_EQ(bs.front(), 10);
        ASSERT_EQ(bs.size(), 2);
        bs.add(1);
        bs.add(2);
        ASSERT_EQ(bs.front(), 1);
        bs.erase(0);
        ASSERT_EQ(bs.size(), 3);
        ASSERT_EQ(bs.front(), 2);

        ASSERT_TRUE(contains(bs, 15));
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

        ASSERT_TRUE(contains(fs, 1));
        join_and_print(fs, std::cout);
        std::cout << '\n';
        join_and_print(fs.complement(), std::cout);
    }

    TEST(Containers, j_and_p) {
        std::vector<int> v {1, 2, 3};
        std::ostringstream ss;
        join_and_print(v, ss);
        ASSERT_EQ(ss.str(), "1, 2, 3");
    }

    TEST(Containers, enumerate) {
        std::vector<uint> v;
        for (uint i = 100; i > 0; --i) {
            v.push_back(i);
        }

        for (auto [i, el] : enumerate(v)) {
            ASSERT_EQ(100 - i, el);
        }
    }

    TEST(random, select_random) {
        std::vector<int> v {0,1,2,3,4,5,6,7,8,9};
        std::mt19937_64 rand(0);
        for (uint i = 0; i < 100; ++i) {
            auto q = selectRandom(v, rand);
            ASSERT_TRUE(q != v.end());
            ASSERT_GE(*q, 0);
            ASSERT_LE(*q, 9);
        }

        for (uint i = 0; i < 100; ++i) {
            auto q = v.begin();
            q = selectRandom(q, v.size(), rand);
            ASSERT_TRUE(q != v.end());
            ASSERT_GE(*q, 0);
            ASSERT_LE(*q, 9);
        }
    }

    TEST(counter, c) {
        Counter c;
        ASSERT_DOUBLE_EQ(c, 0.0);
        ++c;
        ASSERT_DOUBLE_EQ(c, 1.0);
        c -= 3;
        ASSERT_DOUBLE_EQ(c, -2.0);

        ASSERT_TRUE(c == -2.0);

        Counter prova("prova");
        ASSERT_DOUBLE_EQ(prova, 0.0);
        prova = 3.0;
        ASSERT_DOUBLE_EQ(prova, 3.0);

        Counter c2("prova");
        ASSERT_DOUBLE_EQ(c2, 3.0);
        ASSERT_DOUBLE_EQ(c2++, 3.0);
        ASSERT_DOUBLE_EQ(c2, 4.0);
        ASSERT_DOUBLE_EQ(++c2, 5.0);

        Counter secondo("secondo");
        secondo = 5.0;
        ASSERT_EQ(c2, secondo);
    }


#ifdef DFERONE_THREAD_SAFE
    TEST(counter, multithread) {
        Counter c("prova");
        c = 0;
        uint n_threads = 500;
#pragma omp parallel num_threads(n_threads) default(none) shared(c)
        {
            ++c;
        }

        ASSERT_DOUBLE_EQ(c, n_threads);
    }
#endif

    TEST(welford, mean) {
        dferone::WelfordAlgorithm wa;
        std::vector<double> x(1000);
        std::iota(x.begin(), x.end(), 0);
        double mean = 0;
        double variance = 0;
        double std_dev = 0;
        double sum = 0;
        for (auto el : x) {
            sum += el;
            wa.addElement(el);
        }
        mean = sum / x.size();
        for (auto el : x) {
            variance += std::pow(el - mean, 2);
        }
        variance /= x.size();
        std_dev = std::sqrt(variance);

        ASSERT_DOUBLE_EQ(mean, wa.getMean());
        ASSERT_DOUBLE_EQ(variance, wa.getVariance());
        ASSERT_DOUBLE_EQ(std_dev, wa.getStdDev());
    }

    TEST(lik_unl, lik) {
        ASSERT_TRUE(likely(0 == 0));
        ASSERT_FALSE(unlikely(0 == 1));
    }

    TEST(Mat, mat) {
        Matrix<int> mat(3, 3);
        ASSERT_EQ(mat(0, 0), 0);
        mat(0, 0) = 1;
        ASSERT_EQ(mat(0, 0), 1);
    }

    TEST(Mat, sym) {
        SymmetricMatrix<int> sym_mat(3, 1);
        ASSERT_EQ(sym_mat(2, 0), 1);
        sym_mat(2, 0) = 3;
        ASSERT_EQ(sym_mat(0, 2), 3);
    }

}