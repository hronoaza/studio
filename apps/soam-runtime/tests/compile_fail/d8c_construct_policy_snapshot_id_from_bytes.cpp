#include "provenance_admissibility.hpp"
int main() {
    AdaptiveMesh::PolicySnapshotId::Bytes bytes{};
    bytes[0]=1;
    AdaptiveMesh::PolicySnapshotId id{bytes};
    return static_cast<int>(id.bytes()[0]);
}
