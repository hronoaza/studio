#include "policy_persistence.hpp"
void misuse(const AdaptiveMesh::ProductionPersistenceStreamHandle& handle) {
    auto& persistence = handle.bridgePersistence();
    (void)persistence;
}
int main() { return 0; }
