#include "production_relationship_source_snapshot.hpp"

void mutate(
    const AdaptiveMesh::ProductionRelationshipSourceSnapshot& snapshot)
{
    snapshot.sourceCaptureId().bytes()[0] = 0x42;
}

int main() {
    return 0;
}
