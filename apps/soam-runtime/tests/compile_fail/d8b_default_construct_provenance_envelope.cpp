#include "production_provenance_envelope.hpp"
int main() {
    AdaptiveMesh::ProductionProvenanceEnvelope envelope{};
    return static_cast<int>(envelope.sourceNodeId());
}
