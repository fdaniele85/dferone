#include <doctest/doctest.h>

#include <dferone/algorithms/GRASP.h>
#include <dferone/algorithms/SolutionConstructor.h>

class DummyInstance {};
class DummySolution {
public:
    explicit DummySolution(const DummyInstance &) {}
    [[nodiscard]] double get_cost() const { return cost; }
    void set_cost(const double c) { cost = c; }

private:
    double cost{100.0};
};
class DummySolutionConstructor : public dferone::algorithms::SolutionConstructor<DummyInstance, DummySolution> {
public:
    [[nodiscard]] DummySolution create_solution(const DummyInstance &instance, std::mt19937 &) override { return DummySolution(instance); }

    [[nodiscard]] std::unique_ptr<dferone::algorithms::SolutionConstructor<DummyInstance, DummySolution>> clone() const override {
        return std::make_unique<DummySolutionConstructor>(*this);
    }
};
class DummyLocalSearch : public dferone::algorithms::LocalSearch<DummySolution> {
public:
    void search(DummySolution &sol, std::mt19937 &) override { sol.set_cost(sol.get_cost() - 1); }
    [[nodiscard]] std::unique_ptr<dferone::algorithms::LocalSearch<DummySolution>> clone() const override {
        return std::make_unique<DummyLocalSearch>(*this);
    }
};

TEST_CASE("GRASP") {
    dferone::algorithms::GRASP<DummyInstance, DummySolution> grasp(DummyInstance(), {});
    grasp.add_solution_constructor(std::make_unique<DummySolutionConstructor>());
    grasp.set_max_iterations(1);
    auto sol = grasp.solve(1);
    CHECK(sol.get_cost() == 100);

    grasp.add_local_search(std::make_unique<DummyLocalSearch>());
    sol = grasp.solve(1);
    CHECK(sol.get_cost() == 99);
}