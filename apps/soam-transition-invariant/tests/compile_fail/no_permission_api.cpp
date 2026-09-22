#include "production_transition_invariant.hpp"
void misuse(AdaptiveMesh::ProductionTransitionInvariantEvaluator& e){ auto x=e.issuePermission(); (void)x; }
int main(){return 0;}
