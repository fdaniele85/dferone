#include <doctest/doctest.h>

#include <dferone/algorithms/GRASP.h>
#include <dferone/algorithms/SolutionConstructor.h>
#include <vector>

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

struct DummyVisitorCall {
    double cost;
    double time;
};

class DummyVisitor : public dferone::algorithms::AlgorithmVisitor<DummySolution> {
public:
    explicit DummyVisitor(std::vector<DummyVisitorCall> &calls) : calls_(calls) {}

    void on_new_best(const DummySolution &, double cost, double current_time) override { calls_.push_back({cost, current_time}); }

private:
    std::vector<DummyVisitorCall> &calls_;
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

TEST_CASE("GRASP notifies the visitor on new global best") {
    dferone::algorithms::GRASP<DummyInstance, DummySolution> grasp(DummyInstance(), {});
    grasp.add_solution_constructor(std::make_unique<DummySolutionConstructor>());
    grasp.add_local_search(std::make_unique<DummyLocalSearch>());
    grasp.set_max_iterations(1);

    std::vector<DummyVisitorCall> calls;
    grasp.set_visitor(std::make_unique<DummyVisitor>(calls));

    const auto sol = grasp.solve(1);

    CHECK(sol.get_cost() == doctest::Approx(99.0));
    CHECK(calls.size() == 1);
    CHECK(calls.front().cost == doctest::Approx(99.0));
    CHECK(calls.front().time >= 0.0);
}
