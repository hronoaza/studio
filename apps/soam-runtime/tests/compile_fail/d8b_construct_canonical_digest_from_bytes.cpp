#include "production_provenance_envelope.hpp"
int main() {
    AdaptiveMesh::CanonicalDigest::Bytes bytes{};
    AdaptiveMesh::CanonicalDigest digest{bytes};
    return static_cast<int>(digest.bytes()[0]);
}
