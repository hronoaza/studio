#include "versioned_interpretation.hpp"
int main() {
    AdaptiveMesh::InterpretationDecisionId id{};
    return static_cast<int>(id.bytes()[0]);
}
