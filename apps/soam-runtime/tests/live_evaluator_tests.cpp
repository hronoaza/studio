#include "production_transition_evaluator.hpp"
#include "system_architecture.hpp"

#include <atomic>
#include <cstdlib>
#include <memory>
#include <thread>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

int main() {
    {
        SpatialAdaptiveMesh mesh(2);
        mesh.addNode(0, {0.0, 0.0, 0.0}, 1.0);
        mesh.addNode(1, {1.0, 0.0, 0.0}, 1.0);
        auto evaluator = mesh.productionTransitionEvaluator();

        require(evaluator.evaluate({0, 1}) ==
                ProductionTransitionEvaluation::no_request);

        mesh.connectNodes(0, 1);

        require(evaluator.evaluate({0, 1}) ==
                ProductionTransitionEvaluation::not_eligible);
        require(evaluator.evaluate({1, 0}) ==
                ProductionTransitionEvaluation::not_eligible);
        require(evaluator.evaluate({0, 0}) ==
                ProductionTransitionEvaluation::not_eligible);
        require(evaluator.evaluate({0, 99}) ==
                ProductionTransitionEvaluation::not_eligible);
    }

    {
        ProductionTransitionEvaluator evaluator = [] {
            auto mesh = std::make_unique<SpatialAdaptiveMesh>(2);
            mesh->addNode(0, {0.0, 0.0, 0.0}, 1.0);
            mesh->addNode(1, {1.0, 0.0, 0.0}, 1.0);
            mesh->connectNodes(0, 1);
            return mesh->productionTransitionEvaluator();
        }();

        require(evaluator.evaluate({0, 1}) ==
                ProductionTransitionEvaluation::not_eligible);
    }

    {
        SpatialAdaptiveMesh mesh(4);
        for (std::size_t i = 0; i < 8; ++i) {
            mesh.addNode(i, {static_cast<double>(i), 0.0, 0.0}, 1.0);
        }
        for (int i = 1; i < 8; ++i) {
            mesh.connectNodes(i - 1, i);
        }

        auto evaluator = mesh.productionTransitionEvaluator();
        std::atomic<bool> stop{false};
        std::atomic<bool> bad{false};

        std::thread reader([&] {
            while (!stop.load()) {
                const auto result = evaluator.evaluate({0, 1});
                if (result == ProductionTransitionEvaluation::eligible_for_authority_consideration) {
                    bad.store(true);
                    return;
                }
            }
        });

        for (int i = 0; i < 50; ++i) {
            mesh.injectExternalShock(i % 8, (i % 2 == 0) ? 1.0 : -1.0);
            mesh.simulationStep();
        }

        stop.store(true);
        reader.join();
        require(!bad.load());
    }

    return 0;
}
