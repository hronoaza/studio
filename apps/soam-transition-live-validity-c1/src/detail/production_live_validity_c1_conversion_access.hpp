#pragma once

#include "production_transition_eligibility.hpp"

namespace AdaptiveMesh::detail {

class ProductionLiveValidityC1ConversionAccess final {
public:
    [[nodiscard]]
    static constexpr FreshnessPrerequisiteEvidence freshness(
        ProductionTransitionRequestBinding context,
        bool satisfied) noexcept
    {
        return FreshnessPrerequisiteEvidence{context, satisfied};
    }

    [[nodiscard]]
    static constexpr RevalidationPrerequisiteEvidence revalidation(
        ProductionTransitionRequestBinding context,
        bool satisfied) noexcept
    {
        return RevalidationPrerequisiteEvidence{context, satisfied};
    }
};

} // namespace AdaptiveMesh::detail
