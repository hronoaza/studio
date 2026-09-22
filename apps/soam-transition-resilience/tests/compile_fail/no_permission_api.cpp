#include "production_transition_resilience.hpp"
void misuse(AdaptiveMesh::ProductionTransitionResilienceEvaluator& e){ auto x=e.issuePermission(); (void)x; }
int main(){return 0;}
