#include <doctest/doctest.h>

#include <dferone/algorithms/ALNS.h>
#include <dferone/algorithms/DestroyMethod.h>
#include <dferone/algorithms/RepairMethod.h>
#include <string>
#include <vector>

namespace alns_test {

class DummyInstance {};
class DummySolution {
public:
    explicit DummySolution(const DummyInstance &) {}

    [[nodiscard]] double get_cost() const { return cost; }

    void decrement_cost() { cost -= 1.0; }

private:
    double cost{10.0};
};

struct DummyParameters {
public:
    unsigned int seed{0U};
    double initial_temperature{100.0};
    double score_decay{0.8};
    double cooling_rate{0.99};
    std::size_t max_iterations{0};
    double max_time{0.0};
    float accepted_score{1.0F};
    float improved_score{2.0F};
    float new_best_score{4.0F};
};

class DummyDestroyMethod : public dferone::algorithms::DestroyMethod<DummySolution> {
public:
    void destroySolution(DummySolution &, std::mt19937 &) override {}

    [[nodiscard]] std::unique_ptr<dferone::algorithms::DestroyMethod<DummySolution>> clone() const override {
        return std::make_unique<DummyDestroyMethod>(*this);
    }
};

class DummyRepairMethod : public dferone::algorithms::RepairMethod<DummySolution> {
public:
    void repairSolution(DummySolution &sol, std::mt19937 &) override { sol.decrement_cost(); }

    [[nodiscard]] std::unique_ptr<dferone::algorithms::RepairMethod<DummySolution>> clone() const override {
        return std::make_unique<DummyRepairMethod>(*this);
    }
};

class RandomRepairMethod : public dferone::algorithms::RepairMethod<DummySolution> {
public:
    void repairSolution(DummySolution &sol, std::mt19937 &mt) override {
        std::uniform_int_distribution<int> dist(1, 3);
        for (auto i = 0; i < dist(mt); ++i) {
            sol.decrement_cost();
        }
    }

    [[nodiscard]] std::unique_ptr<dferone::algorithms::RepairMethod<DummySolution>> clone() const override {
        return std::make_unique<RandomRepairMethod>(*this);
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

} // namespace alns_test

TEST_CASE("ALNS returns the start solution if no heuristics are configured") {
    dferone::algorithms::ALNS<alns_test::DummyInstance, alns_test::DummySolution, alns_test::DummyParameters> alns(
        alns_test::DummyInstance(), {});
    auto sol = alns.search(1);
    CHECK(sol.get_cost() == doctest::Approx(10.0));
}

TEST_CASE("ALNS keeps each thread state local and only shares the global best") {
    alns_test::DummyParameters parameters;
    parameters.max_iterations = 2;

    dferone::algorithms::ALNS<alns_test::DummyInstance, alns_test::DummySolution, alns_test::DummyParameters> alns(
        alns_test::DummyInstance(), parameters);
    alns.add_destroy_method(std::make_unique<alns_test::DummyDestroyMethod>());
    alns.add_repair_method(std::make_unique<alns_test::DummyRepairMethod>());

    const auto sol = alns.search(4);

    CHECK(sol.get_cost() == doctest::Approx(8.0));
}

TEST_CASE("ALNS derives deterministic thread generators from the master seed") {
    alns_test::DummyParameters parameters;
    parameters.seed = 12345U;
    parameters.max_iterations = 3;

    dferone::algorithms::ALNS<alns_test::DummyInstance, alns_test::DummySolution, alns_test::DummyParameters> first(
        alns_test::DummyInstance(), parameters);
    first.add_destroy_method(std::make_unique<alns_test::DummyDestroyMethod>());
    first.add_repair_method(std::make_unique<alns_test::RandomRepairMethod>());

    dferone::algorithms::ALNS<alns_test::DummyInstance, alns_test::DummySolution, alns_test::DummyParameters> second(
        alns_test::DummyInstance(), parameters);
    second.add_destroy_method(std::make_unique<alns_test::DummyDestroyMethod>());
    second.add_repair_method(std::make_unique<alns_test::RandomRepairMethod>());

    const auto first_sol = first.search(4);
    const auto second_sol = second.search(4);

    CHECK(first_sol.get_cost() == doctest::Approx(second_sol.get_cost()));
}

TEST_CASE("ALNS logs best-solution updates when a logger is configured") {
    alns_test::DummyParameters parameters;
    parameters.max_iterations = 1;

    dferone::algorithms::ALNS<alns_test::DummyInstance, alns_test::DummySolution, alns_test::DummyParameters> alns(
        alns_test::DummyInstance(), parameters);
    alns.add_destroy_method(std::make_unique<alns_test::DummyDestroyMethod>());
    alns.add_repair_method(std::make_unique<alns_test::DummyRepairMethod>());

    std::vector<std::string> logs;
    alns.set_logger([&logs](std::string_view message) { logs.emplace_back(message); });

    const auto sol = alns.search(1);

    CHECK(sol.get_cost() == doctest::Approx(9.0));
    CHECK(logs.size() == 1);
    CHECK(logs.front().find("updating best solution to") != std::string::npos);
}

TEST_CASE("ALNS notifies the visitor on new global best") {
    alns_test::DummyParameters parameters;
    parameters.max_iterations = 1;

    dferone::algorithms::ALNS<alns_test::DummyInstance, alns_test::DummySolution, alns_test::DummyParameters> alns(
        alns_test::DummyInstance(), parameters);
    alns.add_destroy_method(std::make_unique<alns_test::DummyDestroyMethod>());
    alns.add_repair_method(std::make_unique<alns_test::DummyRepairMethod>());

    std::vector<alns_test::DummyVisitorCall> calls;
    alns.set_visitor(std::make_unique<alns_test::DummyVisitor>(calls));

    const auto sol = alns.search(1);

    CHECK(sol.get_cost() == doctest::Approx(9.0));
    CHECK(calls.size() == 1);
    CHECK(calls.front().cost == doctest::Approx(9.0));
    CHECK(calls.front().time >= 0.0);
}
