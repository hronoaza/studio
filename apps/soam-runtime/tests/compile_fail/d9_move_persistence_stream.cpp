#include "policy_persistence.hpp"
#include <utility>
void misuse(AdaptiveMesh::ProductionBridgePersistenceStream& source) {
    AdaptiveMesh::ProductionBridgePersistenceStream moved{std::move(source)};
    (void)moved;
}
int main() { return 0; }
