#include "production_provenance_envelope.hpp"
int main() {
    AdaptiveMesh::ProvenanceItemId::Bytes bytes{};
    bytes[0]=1;
    AdaptiveMesh::ProvenanceItemId id{bytes};
    return static_cast<int>(id.bytes()[0]);
}
