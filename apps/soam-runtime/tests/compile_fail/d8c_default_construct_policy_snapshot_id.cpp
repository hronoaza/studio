#include "provenance_admissibility.hpp"
int main() {
    AdaptiveMesh::PolicySnapshotId id{};
    return static_cast<int>(id.bytes()[0]);
}
