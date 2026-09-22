#include "production_transition_invariant.hpp"
void misuse(AdaptiveMesh::ProductionTransitionInvariantEvaluator& e){ auto x=e.evaluateEligibility(); (void)x; }
int main(){return 0;}
