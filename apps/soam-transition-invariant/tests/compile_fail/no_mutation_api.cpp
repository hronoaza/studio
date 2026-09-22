#include "production_transition_invariant_snapshot.hpp"
void misuse(AdaptiveMesh::ProductionTransitionInvariantSnapshotSource& s){ s.mutateBridge(0,1); }
int main(){return 0;}
