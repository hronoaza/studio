#include "versioned_interpretation.hpp"
int main() {
    AdaptiveMesh::InterpretationDecisionId::Bytes bytes{};
    bytes[0]=1;
    AdaptiveMesh::InterpretationDecisionId id{bytes};
    return static_cast<int>(id.bytes()[0]);
}
