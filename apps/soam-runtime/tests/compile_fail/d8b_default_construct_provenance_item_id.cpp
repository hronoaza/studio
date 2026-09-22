#include "production_provenance_envelope.hpp"
int main() {
    AdaptiveMesh::ProvenanceItemId id{};
    return static_cast<int>(id.bytes()[0]);
}
