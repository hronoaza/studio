#include "provenance_admissibility.hpp"
int main() {
    AdaptiveMesh::AdmissibilityDecisionId id{};
    return static_cast<int>(id.bytes()[0]);
}
