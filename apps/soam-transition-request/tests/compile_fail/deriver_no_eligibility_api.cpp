#include "production_transition_request.hpp"
void misuse(AdaptiveMesh::ProductionTransitionRequestDeriver d, const AdaptiveMesh::ProductionPersistentBridgeRecommendation& r, const AdaptiveMesh::TransitionRequestPolicySnapshot& p){ auto value=d.evaluateEligibility(r,p); (void)value; }
int main(){return 0;}
