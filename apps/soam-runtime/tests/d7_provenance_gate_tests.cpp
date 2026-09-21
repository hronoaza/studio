#include "production_transition_evaluator.hpp"
#include "system_architecture.hpp"

#include <cstdlib>
#include <type_traits>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

static_assert(!std::is_default_constructible_v<ProductionTransitionEvaluator>);

int main() {
    SpatialAdaptiveMesh mesh(2);
    mesh.addNode(0, {0.0, 0.0, 0.0}, 1.0);
    mesh.addNode(1, {1.0, 0.0, 0.0}, 1.0);

    auto evaluator = mesh.productionTransitionEvaluator();

    require(evaluator.evaluate({0, 1}) ==
            ProductionTransitionEvaluation::no_request);

    mesh.connectNodes(0, 1);

    for (int i = 0; i < 32; ++i) {
        const auto result = evaluator.evaluate({0, 1});
        require(result !=
                ProductionTransitionEvaluation::eligible_for_authority_consideration);

        mesh.injectExternalShock(0, (i % 2 == 0) ? 1.0 : -1.0);
        mesh.simulationStep();
    }

    require(evaluator.evaluate({0, 1}) ==
            ProductionTransitionEvaluation::not_eligible);

    return 0;
}
