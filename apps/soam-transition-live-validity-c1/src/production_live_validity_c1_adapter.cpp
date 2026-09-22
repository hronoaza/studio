#include "production_live_validity_c1_adapter.hpp"
#include "detail/production_live_validity_c1_conversion_access.hpp"

namespace AdaptiveMesh {

std::optional<ProductionLiveValidityC1Prerequisites>
ProductionLiveValidityC1Adapter::convert(
    const ProductionTransitionLiveValidityEvidence& evidence) const noexcept
{
    const auto& requestBinding = evidence.request().binding();
    const auto& freshnessRecord = evidence.freshness();
    const auto& revalidationRecord = evidence.revalidation();

    if (freshnessRecord.binding() != requestBinding ||
        revalidationRecord.binding() != requestBinding) {
        return std::nullopt;
    }

    return ProductionLiveValidityC1Prerequisites{
        evidence.decisionId(),
        freshnessRecord.decisionId(),
        revalidationRecord.decisionId(),
        detail::ProductionLiveValidityC1ConversionAccess::freshness(
            freshnessRecord.binding(),
            freshnessRecord.satisfied()),
        detail::ProductionLiveValidityC1ConversionAccess::revalidation(
            revalidationRecord.binding(),
            revalidationRecord.satisfied())
    };
}

} // namespace AdaptiveMesh
