#include "production_transition_invariant_snapshot.hpp"
#include "system_architecture.hpp"

#include <cstdlib>
#include <optional>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

int main() {
    std::optional<ProductionTransitionInvariantSnapshotSource> staleSource;

    {
        SpatialAdaptiveMesh mesh(1);
        mesh.addNode(0,{0.0,0.0,0.0},1.0);
        mesh.addNode(1,{0.0,0.0,1.0},1.0);
        mesh.connectNodes(0,1);

        auto source = mesh.productionTransitionInvariantSnapshotSource();
        staleSource.emplace(source);

        const auto snapshot = source.capture(0,1);
        require(snapshot.has_value());
        require(snapshot->sourceNodeId() == 0U);
        require(snapshot->targetNodeId() == 1U);
        require(snapshot->stateVersion() > 0U);
        require(snapshot->forwardBridge().has_value());
        require(snapshot->reverseBridge().has_value());

        const auto& forward = *snapshot->forwardBridge();
        const auto& reverse = *snapshot->reverseBridge();
        require(forward.targetNodeId() == 1U);
        require(reverse.targetNodeId() == 0U);
        require(forward.generation() == reverse.generation());
        require(forward.distance() == reverse.distance());
        require(forward.capacity() == 1.0);
        require(reverse.capacity() == 1.0);
        require(snapshot->sourceInvariantMaxEpsilon() > 0.0);
        require(snapshot->targetInvariantMaxEpsilon() > 0.0);

        require(!source.capture(0,0).has_value());
        require(!source.capture(0,2).has_value());
    }

    require(staleSource.has_value());
    require(!staleSource->capture(0,1).has_value());
    return 0;
}
