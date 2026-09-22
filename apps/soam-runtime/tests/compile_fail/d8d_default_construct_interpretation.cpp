#include "versioned_interpretation.hpp"
int main() {
    AdaptiveMesh::VersionedProductionInterpretation value{};
    return static_cast<int>(value.observation().compatibility());
}
