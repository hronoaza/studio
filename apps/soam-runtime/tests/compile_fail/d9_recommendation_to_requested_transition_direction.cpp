#include "policy_persistence.hpp"
#include "production_transition_eligibility.hpp"

void misuse(const AdaptiveMesh::ProductionPersistentBridgeRecommendation& rec) {
    AdaptiveMesh::RequestedTransitionDirection direction = rec.recommendation();
    (void)direction;
}

int main() { return 0; }
