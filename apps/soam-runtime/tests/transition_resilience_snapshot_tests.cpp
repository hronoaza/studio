#include "production_transition_resilience_snapshot.hpp"
#include "system_architecture.hpp"

#include <cstdlib>
#include <optional>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

int main() {
    std::optional<ProductionTransitionResilienceSnapshotSource> staleSource;

    {
        SpatialAdaptiveMesh mesh(1);
        mesh.addNode(0,{0.0,0.0,0.0},1.0);
        mesh.addNode(1,{0.0,0.0,1.0},1.0);
        mesh.addNode(2,{1.0,0.0,0.0},1.0);
        mesh.connectNodePairs({{0,1},{0,2},{2,1}});

        auto source=mesh.productionTransitionResilienceSnapshotSource();
        staleSource.emplace(source);

        const auto snapshot=source.capture();
        require(snapshot.has_value());
        require(snapshot->stateVersion()>0U);
        require(snapshot->nodeIds().size()==3U);
        require(snapshot->relationships().size()==3U);

        for (const auto& relationship:snapshot->relationships()) {
            require(relationship.nodeA()<relationship.nodeB());
            require(relationship.aToB().size()==1U);
            require(relationship.bToA().size()==1U);
            const auto& f=relationship.aToB().front();
            const auto& r=relationship.bToA().front();
            require(f.sourceNodeId()==relationship.nodeA());
            require(f.targetNodeId()==relationship.nodeB());
            require(r.sourceNodeId()==relationship.nodeB());
            require(r.targetNodeId()==relationship.nodeA());
            require(f.generation()==r.generation());
            require(f.distance()==r.distance());
        }
    }

    require(staleSource.has_value());
    require(!staleSource->capture().has_value());
    return 0;
}
