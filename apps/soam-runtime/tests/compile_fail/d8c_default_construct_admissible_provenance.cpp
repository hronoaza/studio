#include "provenance_admissibility.hpp"
int main() {
    AdaptiveMesh::AdmissibleProductionProvenance value{};
    return static_cast<int>(value.decisionId().bytes()[0]);
}
