#include "policy_persistence.hpp"
void misuse(
    AdaptiveMesh::PersistenceStreamInstanceId instanceId,
    AdaptiveMesh::ProductionPersistenceStreamKey key,
    AdaptiveMesh::PersistencePolicySnapshot policy)
{
    AdaptiveMesh::ProductionBridgePersistenceStream stream{
        instanceId, key, policy, 1U};
    (void)stream;
}
int main() { return 0; }
