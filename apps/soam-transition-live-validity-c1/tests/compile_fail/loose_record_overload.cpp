#include "production_live_validity_c1_adapter.hpp"

void misuse(
    const AdaptiveMesh::ProductionFreshnessPrerequisiteRecord& freshness,
    const AdaptiveMesh::ProductionRevalidationPrerequisiteRecord& revalidation)
{
    AdaptiveMesh::ProductionLiveValidityC1Adapter adapter;
    (void)adapter.convert(freshness, revalidation);
}
