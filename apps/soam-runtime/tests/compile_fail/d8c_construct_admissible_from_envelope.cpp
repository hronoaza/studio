#include "provenance_admissibility.hpp"
void forge(const AdaptiveMesh::ProductionProvenanceEnvelope& envelope) {
    AdaptiveMesh::AdmissibleProductionProvenance value{envelope};
    static_cast<void>(value);
}
int main() { return 0; }
