#include "production_transition_eligibility.hpp"

#include <cstdint>
#include <cstdlib>
#include <type_traits>

namespace AdaptiveMesh::detail {

class ProductionTransitionConstructionAccess final {
public:
    static constexpr ProductionRelationshipIdentity relationship(
        std::size_t sourceNodeId,
        std::size_t targetNodeId,
        std::uint64_t generation) noexcept
    {
        return ProductionRelationshipIdentity{
            sourceNodeId, targetNodeId, generation};
    }

    static constexpr ProductionStateVersion stateVersion(
        std::uint64_t value) noexcept
    {
        return ProductionStateVersion{value};
    }

    static constexpr ProductionTransitionClassId transitionClass(
        std::uint64_t value) noexcept
    {
        return ProductionTransitionClassId{value};
    }

    static constexpr ProductionTransitionRequestBinding binding(
        std::size_t sourceNodeId,
        std::size_t targetNodeId,
        std::uint64_t generation,
        RequestedTransitionDirection direction,
        std::uint64_t transitionClassId,
        std::uint64_t stateVersionValue) noexcept
    {
        return ProductionTransitionRequestBinding{
            relationship(sourceNodeId, targetNodeId, generation),
            direction,
            transitionClass(transitionClassId),
            stateVersion(stateVersionValue)};
    }

    static constexpr PermissionPrerequisiteEvidence permission(
        ProductionTransitionRequestBinding binding,
        bool satisfied) noexcept
    {
        return PermissionPrerequisiteEvidence{binding, satisfied};
    }

    static constexpr InvariantPrerequisiteEvidence invariant(
        ProductionTransitionRequestBinding binding,
        bool satisfied) noexcept
    {
        return InvariantPrerequisiteEvidence{binding, satisfied};
    }

    static constexpr ResiliencePrerequisiteEvidence resilience(
        ProductionTransitionRequestBinding binding,
        bool satisfied) noexcept
    {
        return ResiliencePrerequisiteEvidence{binding, satisfied};
    }

    static constexpr FreshnessPrerequisiteEvidence freshness(
        ProductionTransitionRequestBinding binding,
        bool satisfied) noexcept
    {
        return FreshnessPrerequisiteEvidence{binding, satisfied};
    }

    static constexpr RevalidationPrerequisiteEvidence revalidation(
        ProductionTransitionRequestBinding binding,
        bool satisfied) noexcept
    {
        return RevalidationPrerequisiteEvidence{binding, satisfied};
    }
};

} // namespace AdaptiveMesh::detail

using namespace AdaptiveMesh;
using Access = AdaptiveMesh::detail::ProductionTransitionConstructionAccess;

static_assert(!std::is_default_constructible_v<ProductionRelationshipIdentity>);
static_assert(!std::is_constructible_v<
    ProductionRelationshipIdentity, std::size_t, std::size_t, std::uint64_t>);
static_assert(!std::is_default_constructible_v<ProductionStateVersion>);
static_assert(!std::is_constructible_v<ProductionStateVersion, std::uint64_t>);
static_assert(!std::is_default_constructible_v<ProductionTransitionClassId>);
static_assert(!std::is_constructible_v<ProductionTransitionClassId, std::uint64_t>);
static_assert(!std::is_default_constructible_v<ProductionTransitionRequestBinding>);
static_assert(!std::is_constructible_v<PermissionPrerequisiteEvidence, bool>);
static_assert(!std::is_constructible_v<InvariantPrerequisiteEvidence, bool>);
static_assert(!std::is_constructible_v<ResiliencePrerequisiteEvidence, bool>);
static_assert(!std::is_constructible_v<FreshnessPrerequisiteEvidence, bool>);
static_assert(!std::is_constructible_v<RevalidationPrerequisiteEvidence, bool>);

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }

ProductionTransitionRequestBinding makeRequest(
    RequestedTransitionDirection direction =
        RequestedTransitionDirection::constrain,
    std::uint64_t generation = 7,
    std::uint64_t transitionClass = 11,
    std::uint64_t stateVersion = 21)
{
    return Access::binding(
        100, 200, generation, direction, transitionClass, stateVersion);
}

ProductionTransitionPrerequisiteSet allSatisfied(
    const ProductionTransitionRequestBinding& request)
{
    return {
        Access::permission(request, true),
        Access::invariant(request, true),
        Access::resilience(request, true),
        Access::freshness(request, true),
        Access::revalidation(request, true)
    };
}

void expect(
    const ProductionTransitionEligibilityDecision& decision,
    ProductionTransitionEligibility eligibility,
    EligibilityRejectionReason reason) noexcept
{
    require(decision.eligibility() == eligibility);
    require(decision.rejectionReason() == reason);
}

int main() {
    const ProductionTransitionEligibilityEvaluator evaluator;
    const auto request = makeRequest();

    {
        const auto prerequisites = allSatisfied(request);
        const auto decision = evaluator.evaluate(request, prerequisites);
        expect(
            decision,
            ProductionTransitionEligibility::eligible_for_authority_consideration,
            EligibilityRejectionReason::none);
        require(decision.binding() == request);
    }

    {
        auto prerequisites = allSatisfied(request);
        prerequisites.permission.reset();
        expect(
            evaluator.evaluate(request, prerequisites),
            ProductionTransitionEligibility::not_eligible,
            EligibilityRejectionReason::missing_prerequisite);
    }

    {
        auto prerequisites = allSatisfied(request);
        const auto other = makeRequest(RequestedTransitionDirection::support);
        prerequisites.permission = Access::permission(other, false);
        expect(
            evaluator.evaluate(request, prerequisites),
            ProductionTransitionEligibility::not_eligible,
            EligibilityRejectionReason::binding_mismatch);
    }

    {
        auto prerequisites = allSatisfied(request);
        prerequisites.permission = Access::permission(request, false);
        prerequisites.invariant = Access::invariant(request, false);
        expect(
            evaluator.evaluate(request, prerequisites),
            ProductionTransitionEligibility::not_eligible,
            EligibilityRejectionReason::permission_not_satisfied);
    }

    {
        auto prerequisites = allSatisfied(request);
        prerequisites.invariant = Access::invariant(request, false);
        prerequisites.resilience = Access::resilience(request, false);
        expect(
            evaluator.evaluate(request, prerequisites),
            ProductionTransitionEligibility::not_eligible,
            EligibilityRejectionReason::invariant_not_satisfied);
    }

    {
        auto prerequisites = allSatisfied(request);
        prerequisites.resilience = Access::resilience(request, false);
        prerequisites.freshness = Access::freshness(request, false);
        expect(
            evaluator.evaluate(request, prerequisites),
            ProductionTransitionEligibility::not_eligible,
            EligibilityRejectionReason::resilience_not_satisfied);
    }

    {
        auto prerequisites = allSatisfied(request);
        prerequisites.freshness = Access::freshness(request, false);
        prerequisites.revalidation = Access::revalidation(request, false);
        expect(
            evaluator.evaluate(request, prerequisites),
            ProductionTransitionEligibility::not_eligible,
            EligibilityRejectionReason::freshness_not_satisfied);
    }

    {
        auto prerequisites = allSatisfied(request);
        prerequisites.revalidation = Access::revalidation(request, false);
        expect(
            evaluator.evaluate(request, prerequisites),
            ProductionTransitionEligibility::not_eligible,
            EligibilityRejectionReason::revalidation_failed);
    }

    {
        const auto first = evaluator.evaluate(request, allSatisfied(request));
        const auto second = evaluator.evaluate(request, allSatisfied(request));
        require(first.eligibility() == second.eligibility());
        require(first.rejectionReason() == second.rejectionReason());
        require(first.binding() == second.binding());
    }

    return 0;
}
