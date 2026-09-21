#include "production_transition_evaluator.hpp"
#include "interaction_observation.hpp"
#include "bridge_confidence.hpp"

int main() {
    AdaptiveMesh::ProductionTransitionEvaluator evaluator{};
    const AdaptiveMesh::InteractionObservation observation{1.0};
    const AdaptiveMesh::BridgeConfidence confidence{1.0};

    // D7 must not expose a caller-supplied provenance injection overload.
    return static_cast<int>(
        evaluator.evaluate({0, 1}, observation, confidence));
}
