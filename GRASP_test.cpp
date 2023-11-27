#include <gtest/gtest.h>

#include "grasp/GRASP.h"
#include "grasp/SolutionConstructor.h"

namespace {
    using namespace dferone::grasp;

    struct Instance {};
    struct Solution {
    public:
        explicit Solution(const Instance & instance, double c = std::numeric_limits<double>::max()) {
            cost_ = c;
        }
        [[nodiscard]] constexpr double getCost() const { return cost_; }
        void update(double x) {
            cost_ += x;
        }
    private:
        double cost_;
    };
    class SC : public SolutionConstructor<Instance, Solution> {
    public:
        Solution createSolution(const Instance& instance, std::mt19937& mt) override {
            std::uniform_real_distribution<double> dis(0, 10);
            Solution s(instance, dis(mt));
            for (uint i = 0; i < 1000000; ++i) {
                auto s = std::sqrt(4);
            }
            return s;
        };
        [[nodiscard]] std::unique_ptr<SolutionConstructor<Instance, Solution>> clone() const override {
            return std::make_unique<SC>();
        };
    };

    struct LS : public LocalSearch<Solution> {
        void search(Solution &s, std::mt19937& mt) override {
            s.update(-std::min(s.getCost(), 1.0));
        };
        [[nodiscard]] std::unique_ptr<LocalSearch<Solution>> clone() const override {
            return std::make_unique<LS>();
        };
    };

    TEST(Grasp, creation) {
        std::mt19937 mt(0);
        Instance instance;
        GRASP<Instance, Solution> g(instance, 0);
        g.setMaxIterations(10);
        ASSERT_ANY_THROW(g.solve(1));
        auto sc = std::make_unique<SC>();
        auto s = sc->createSolution(instance, mt);
        ASSERT_GE(s.getCost(), 0.0);
        ASSERT_LE(s.getCost(), 10.0);
        g.addSolutionConstructor(std::move(sc));
        g.addLocalSearch(std::make_unique<LS>());
        s = g.solve(3);
        ASSERT_GE(s.getCost(), 0.0);
        ASSERT_LE(s.getCost(), 10.0);
        GRASP<Instance, Solution> g2(instance, 0);
        g2.addSolutionConstructor(std::make_unique<SC>());
        g2.addLocalSearch(std::make_unique<LS>());
        g2.setMaxIterations(150);
        auto s2 = g2.solve(3);
        ASSERT_LE(s2.getCost(), s.getCost());
    }
}