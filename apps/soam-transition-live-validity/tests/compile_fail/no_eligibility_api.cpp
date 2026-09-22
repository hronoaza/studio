#include "production_transition_live_validity.hpp"
void misuse(AdaptiveMesh::ProductionTransitionLiveValidityEvaluator& e,const AdaptiveMesh::ProductionDerivedTransitionRequest& r){ auto x=e.evaluateEligibility(r); (void)x; }
int main(){return 0;}
