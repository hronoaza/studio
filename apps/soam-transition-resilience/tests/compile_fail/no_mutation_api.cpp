#include "production_transition_resilience_snapshot.hpp"
void misuse(AdaptiveMesh::ProductionTransitionResilienceSnapshotSource& s){ s.removeRelationship(0,1); }
int main(){return 0;}
