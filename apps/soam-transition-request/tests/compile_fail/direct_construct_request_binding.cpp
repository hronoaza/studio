#include "production_transition_eligibility.hpp"
void misuse(AdaptiveMesh::ProductionRelationshipIdentity r, AdaptiveMesh::ProductionTransitionClassId c, AdaptiveMesh::ProductionStateVersion v){ AdaptiveMesh::ProductionTransitionRequestBinding b{r,AdaptiveMesh::RequestedTransitionDirection::support,c,v}; (void)b; }
int main(){return 0;}
