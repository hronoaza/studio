#include "policy_persistence.hpp"
void misuse(
    AdaptiveMesh::PersistencePolicySnapshotId snapshotId,
    AdaptiveMesh::PersistencePolicyDescriptor descriptor)
{
    AdaptiveMesh::PersistencePolicySnapshot policy{
        snapshotId, descriptor, 0.90, 0.10, 7U, 7U};
    (void)policy;
}
int main() { return 0; }
