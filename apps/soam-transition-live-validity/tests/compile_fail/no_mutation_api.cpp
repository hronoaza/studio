#include "production_transition_live_snapshot.hpp"
void misuse(AdaptiveMesh::ProductionTransitionLiveSnapshotSource& s){ s.mutateBridge(0,1); }
int main(){return 0;}
