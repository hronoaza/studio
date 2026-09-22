#include "provenance_admissibility.hpp"
int main() {
    AdaptiveMesh::AdmissibilityDecisionId::Bytes bytes{};
    bytes[0]=1;
    AdaptiveMesh::AdmissibilityDecisionId id{bytes};
    return static_cast<int>(id.bytes()[0]);
}
