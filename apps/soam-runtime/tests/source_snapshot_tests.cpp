#include "production_relationship_source_snapshot.hpp"
#include "system_architecture.hpp"

#include <cmath>
#include <cstdlib>
#include <type_traits>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }
void requireNear(double actual, double expected, double epsilon = 1e-12) noexcept {
    if (std::abs(actual - expected) > epsilon) fail();
}

static_assert(!std::is_default_constructible_v<
    ProductionRelationshipSourceSnapshot>);
static_assert(!std::is_constructible_v<
    ProductionRelationshipSourceSnapshot,
    std::size_t,
    std::size_t,
    std::uint64_t,
    std::uint64_t,
    double,
    double,
    double,
    BridgeStatus,
    double,
    double,
    double,
    double>);

int main() {
    SpatialAdaptiveMesh mesh(2);
    mesh.addNode(0, {0.0, 0.0, 0.0}, 1.0);
    mesh.addNode(1, {1.0, 0.0, 0.0}, 1.0);

    require(!mesh.captureProductionRelationshipSourceSnapshot({0, 1}).has_value());
    require(!mesh.captureProductionRelationshipSourceSnapshot({0, 0}).has_value());
    require(!mesh.captureProductionRelationshipSourceSnapshot({0, 99}).has_value());

    mesh.connectNodes(0, 1);

    const auto first =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(first.has_value());

    require(first->sourceNodeId() == 0);
    require(first->targetNodeId() == 1);
    require(first->relationshipGeneration() > 0);
    require(first->stateVersion() > 0);
    requireNear(first->distance(), 1.0);
    requireNear(first->orientationWeight(), 0.5);
    requireNear(first->capacity(), 1.0);
    require(first->bridgeStatus() == BridgeStatus::NORMAL);
    requireNear(first->sourceState(), 1.0);
    requireNear(first->targetState(), 1.0);
    requireNear(first->sourceHealth(), 1.0);
    requireNear(first->targetHealth(), 1.0);

    const auto same =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(same.has_value());
    require(same->relationshipGeneration() == first->relationshipGeneration());
    require(same->stateVersion() == first->stateVersion());

    const auto reverse =
        mesh.captureProductionRelationshipSourceSnapshot({1, 0});
    require(reverse.has_value());
    require(reverse->relationshipGeneration() == first->relationshipGeneration());
    requireNear(reverse->distance(), first->distance());
    requireNear(reverse->orientationWeight(), 0.5);

    mesh.injectExternalShock(0, 1.0);

    const auto afterShock =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(afterShock.has_value());
    require(afterShock->relationshipGeneration() == first->relationshipGeneration());
    require(afterShock->stateVersion() > first->stateVersion());
    require(afterShock->sourceState() != first->sourceState());

    mesh.simulationStep();

    const auto afterStep =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(afterStep.has_value());
    require(afterStep->relationshipGeneration() == first->relationshipGeneration());
    require(afterStep->stateVersion() > afterShock->stateVersion());

    return 0;
}
