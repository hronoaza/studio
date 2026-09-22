#include "production_transition_live_snapshot.hpp"
#include "system_architecture.hpp"

#include <cstdlib>
#include <optional>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

int main() {
    std::optional<ProductionTransitionLiveSnapshotSource> staleSource;

    {
        SpatialAdaptiveMesh mesh(2);
        mesh.addNode(0,{0.0,0.0,0.0},1.0);
        mesh.addNode(1,{0.0,0.0,1.0},1.0);
        mesh.connectNodes(0,1);

        auto source = mesh.productionTransitionLiveSnapshotSource();
        staleSource.emplace(source);

        const auto present = source.capture(0,1);
        require(present.has_value());
        require(present->sourceNodeId() == 0U);
        require(present->targetNodeId() == 1U);
        require(present->relationshipPresent());
        require(present->relationshipGeneration().has_value());
        const auto generation = *present->relationshipGeneration();
        const auto stateVersion = present->stateVersion();

        mesh.pruneIsolatedBridges(1.1);

        const auto absent = source.capture(0,1);
        require(absent.has_value());
        require(absent->sourceNodeId() == 0U);
        require(absent->targetNodeId() == 1U);
        require(!absent->relationshipPresent());
        require(!absent->relationshipGeneration().has_value());
        require(absent->stateVersion() > stateVersion);

        mesh.connectNodes(0,1);
        const auto recreated = source.capture(0,1);
        require(recreated.has_value());
        require(recreated->relationshipPresent());
        require(recreated->relationshipGeneration().has_value());
        require(*recreated->relationshipGeneration() != generation);

        require(!source.capture(0,0).has_value());
        require(!source.capture(0,2).has_value());
    }

    require(staleSource.has_value());
    require(!staleSource->capture(0,1).has_value());

    return 0;
}
