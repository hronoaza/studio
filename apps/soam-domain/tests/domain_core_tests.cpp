#include "interaction_observation.hpp"
#include "bridge_confidence.hpp"
#include "adaptive_bridge_policy.hpp"
#include "bridge_persistence.hpp"

#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename Exception, typename Function>
void requireThrows(Function&& function, const char* message) {
    try {
        function();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error(message);
}

void test_observation_and_confidence_contracts() {
    using namespace AdaptiveMesh;

    static_assert(!std::is_default_constructible_v<InteractionObservation>);
    static_assert(!std::is_convertible_v<double, InteractionObservation>);
    static_assert(!std::is_default_constructible_v<BridgeConfidence>);
    static_assert(!std::is_convertible_v<double, BridgeConfidence>);

    require(InteractionObservation(0.0).compatibility() == 0.0,
            "zero observation must be preserved");
    require(InteractionObservation(1.0).compatibility() == 1.0,
            "unit observation must be preserved");
    require(BridgeConfidence(0.0).value() == 0.0,
            "zero confidence must be preserved");
    require(BridgeConfidence(1.0).value() == 1.0,
            "unit confidence must be preserved");

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();

    requireThrows<std::invalid_argument>(
        [nan] { static_cast<void>(InteractionObservation(nan)); },
        "NaN observation must be rejected");
    requireThrows<std::invalid_argument>(
        [infinity] { static_cast<void>(InteractionObservation(infinity)); },
        "infinite observation must be rejected");
    requireThrows<std::invalid_argument>(
        [] { static_cast<void>(InteractionObservation(-0.001)); },
        "negative observation must be rejected");
    requireThrows<std::invalid_argument>(
        [] { static_cast<void>(InteractionObservation(1.001)); },
        "observation above one must be rejected");

    requireThrows<std::invalid_argument>(
        [nan] { static_cast<void>(BridgeConfidence(nan)); },
        "NaN confidence must be rejected");
    requireThrows<std::invalid_argument>(
        [infinity] { static_cast<void>(BridgeConfidence(infinity)); },
        "infinite confidence must be rejected");
    requireThrows<std::invalid_argument>(
        [] { static_cast<void>(BridgeConfidence(-0.001)); },
        "negative confidence must be rejected");
    requireThrows<std::invalid_argument>(
        [] { static_cast<void>(BridgeConfidence(1.001)); },
        "confidence above one must be rejected");
}

void test_policy_evidence_contract() {
    using namespace AdaptiveMesh;

    static_assert(!std::is_default_constructible_v<BridgePolicyEvidence>);
    static_assert(!std::is_constructible_v<BridgePolicyEvidence, double>);
    static_assert(noexcept(
        std::declval<const AdaptiveBridgePolicy&>().evaluate(
            std::declval<const InteractionObservation&>(),
            std::declval<const BridgeConfidence&>())));

    const AdaptiveBridgePolicy policy;
    const BridgeConfidence fullConfidence(1.0);

    require(policy.evaluate(InteractionObservation(0.0), fullConfidence).value() == -1.0,
            "zero compatibility must map to full negative evidence");
    require(policy.evaluate(InteractionObservation(0.5), fullConfidence).value() == 0.0,
            "neutral compatibility must map to zero evidence");
    require(policy.evaluate(InteractionObservation(1.0), fullConfidence).value() == 1.0,
            "unit compatibility must map to full positive evidence");

    require(policy.evaluate(
                InteractionObservation(1.0), BridgeConfidence(0.0)).value() == 0.0,
            "zero confidence must suppress evidence");

    const double positiveLow =
        policy.evaluate(InteractionObservation(0.75), BridgeConfidence(0.4)).value();
    const double positiveHigh =
        policy.evaluate(InteractionObservation(0.75), BridgeConfidence(0.8)).value();
    const double negativeHigh =
        policy.evaluate(InteractionObservation(0.25), BridgeConfidence(0.8)).value();

    require(positiveHigh > positiveLow,
            "positive evidence magnitude must increase with confidence");
    require(std::abs(negativeHigh + positiveHigh) < 1e-12,
            "complementary observations must be symmetric");
    require(negativeHigh >= -1.0 && positiveHigh <= 1.0,
            "evidence must stay bounded");
}

void test_persistence_contract() {
    using namespace AdaptiveMesh;

    static_assert(!std::is_default_constructible_v<BridgePersistence>);
    static_assert(noexcept(std::declval<BridgePersistence&>().observe(
        std::declval<const BridgePolicyEvidence&>())));
    static_assert(noexcept(std::declval<BridgePersistence&>().reset()));

    requireThrows<std::invalid_argument>(
        [] { static_cast<void>(BridgePersistence(0.5, 0.5, 2, 2)); },
        "release threshold must be lower than activation threshold");
    requireThrows<std::invalid_argument>(
        [] { static_cast<void>(BridgePersistence(0.5, 0.2, 1, 2)); },
        "activation sample count below two must be rejected");
    requireThrows<std::invalid_argument>(
        [] { static_cast<void>(BridgePersistence(0.5, 0.2, 2, 1)); },
        "release sample count below two must be rejected");

    const AdaptiveBridgePolicy policy;
    const BridgeConfidence fullConfidence(1.0);
    const auto evidence = [&policy, &fullConfidence](double compatibility) {
        return policy.evaluate(
            InteractionObservation(compatibility), fullConfidence);
    };

    BridgePersistence support(0.5, 0.25, 2, 2);
    require(support.observe(evidence(1.0)) ==
                PersistentBridgeRecommendation::PRESERVE,
            "first support sample must not activate");
    require(support.observe(evidence(1.0)) ==
                PersistentBridgeRecommendation::SUPPORT,
            "second support sample must activate");
    require(support.observe(evidence(0.625)) ==
                PersistentBridgeRecommendation::SUPPORT,
            "first release-boundary sample must preserve support");
    require(support.observe(evidence(0.625)) ==
                PersistentBridgeRecommendation::PRESERVE,
            "second release-boundary sample must release");

    BridgePersistence constrain(0.5, 0.25, 2, 2);
    require(constrain.observe(evidence(0.0)) ==
                PersistentBridgeRecommendation::PRESERVE,
            "first constrain sample must not activate");
    require(constrain.observe(evidence(0.0)) ==
                PersistentBridgeRecommendation::CONSTRAIN,
            "second constrain sample must activate");

    BridgePersistence reversal(0.5, 0.25, 2, 2);
    require(reversal.observe(evidence(1.0)) ==
                PersistentBridgeRecommendation::PRESERVE,
            "positive pending streak must start");
    require(reversal.observe(evidence(0.0)) ==
                PersistentBridgeRecommendation::PRESERVE,
            "direction reversal must reset pending streak");
    require(reversal.observe(evidence(0.0)) ==
                PersistentBridgeRecommendation::CONSTRAIN,
            "new direction must require its own confirmation");

    support.reset();
    require(support.observe(evidence(1.0)) ==
                PersistentBridgeRecommendation::PRESERVE,
            "reset must clear confirmed state");

    BridgePersistence isolatedA(0.5, 0.25, 2, 2);
    BridgePersistence isolatedB(0.5, 0.25, 2, 2);
    require(isolatedA.observe(evidence(1.0)) ==
                PersistentBridgeRecommendation::PRESERVE,
            "instance A first sample");
    require(isolatedB.observe(evidence(1.0)) ==
                PersistentBridgeRecommendation::PRESERVE,
            "instance B first sample");
    require(isolatedA.observe(evidence(1.0)) ==
                PersistentBridgeRecommendation::SUPPORT,
            "instance A must confirm independently");
    require(isolatedB.observe(evidence(0.5)) ==
                PersistentBridgeRecommendation::PRESERVE,
            "instance B state must remain independent");
}

} // namespace

int main() {
    test_observation_and_confidence_contracts();
    test_policy_evidence_contract();
    test_persistence_contract();
    return 0;
}
