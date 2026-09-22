#include "production_transition_live_validity.hpp"
void misuse(AdaptiveMesh::ProductionTransitionLiveValidityEvaluator& e){ auto x=e.issuePermission(); (void)x; }
int main(){return 0;}
