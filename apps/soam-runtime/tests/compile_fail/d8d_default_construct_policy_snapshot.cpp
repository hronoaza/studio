#include "versioned_interpretation.hpp"
int main() {
    AdaptiveMesh::InterpretationPolicySnapshot snapshot{};
    return snapshot.directional() ? 1 : 0;
}
