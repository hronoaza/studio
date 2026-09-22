#include "production_live_validity_c1_adapter.hpp"

void misuse(
    const AdaptiveMesh::ProductionTransitionLiveValidityEvidence& evidence)
{
    AdaptiveMesh::ProductionLiveValidityC1Adapter adapter;
    (void)adapter.evaluateEligibility(evidence);
}
