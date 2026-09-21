#include "production_relationship_provenance.hpp"\n#include "system_architecture.hpp"

int main() {
    AdaptiveMesh::ProductionRelationshipProvenance forged{
        0,
        1,
        7,
        11,
        1.0,
        0.5,
        1.0,
        AdaptiveMesh::BridgeStatus::NORMAL,
        1.0,
        1.0,
        1.0,
        1.0
    };
    return static_cast<int>(forged.relationshipGeneration());
}
