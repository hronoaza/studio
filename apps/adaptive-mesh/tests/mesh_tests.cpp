#include <cassert>
#include <cmath>
#include <iostream>
#include "system_architecture.hpp"

using namespace AdaptiveMesh;

void test_vector_geometry() {
    Vector3D a{0.0, 0.0, 0.0};
    Vector3D b{2.0, 2.0, 2.0};
    assert(std::abs(a.distanceTo(b) - std::sqrt(12.0)) < 1e-12);
    assert(a.distanceTo(b) > 3.0);
    assert(a.distanceTo(b) < 3.5);
}

void test_identity_and_meta_evaluator() {
    IdentityInvariant invariant;
    invariant.baseline = 1.0;
    invariant.maxEpsilon = 10.0;

    assert(invariant.isWithinSafetyBound(11.0));
    assert(!invariant.isWithinSafetyBound(11.0001));

    MetaEvaluator evaluator;
    assert(evaluator.evaluate(1.5, 1.0, invariant) == SignalCategory::NOISE);
    assert(evaluator.evaluate(2.5, 0.9, invariant) == SignalCategory::CREATIVE_SIGNAL);
    assert(evaluator.evaluate(20.0, 1.0, invariant) == SignalCategory::DESTRUCTIVE_DRIFT);
}

void test_bridge_state_machine() {
    SpatialBridge bridge{1, 1.0, 1.0, 1.0, BridgeStatus::NORMAL};

    bridge.updateBridgeState(static_cast<double>(SignalCategory::NOISE));
    assert(bridge.status == BridgeStatus::DAMPING);
    assert(std::abs(bridge.capacity - 0.85) < 1e-12);

    bridge.updateBridgeState(static_cast<double>(SignalCategory::CREATIVE_SIGNAL));
    assert(bridge.status == BridgeStatus::NORMAL);
    assert(std::abs(bridge.capacity - 1.0) < 1e-12);

    bridge.updateBridgeState(static_cast<double>(SignalCategory::DESTRUCTIVE_DRIFT));
    assert(bridge.status == BridgeStatus::ISOLATED);
    assert(bridge.capacity == 0.0);
    assert(bridge.getEffectiveTransmission() == 0.0);
}

void test_local_reflex_filter() {
    AutopoieticNode node(0, {0.0, 0.0, 0.0}, 1.0);

    assert(std::abs(node.applyLocalReflexFilter(2.0) - 2.0) < 1e-12);
    assert(std::abs(node.applyLocalReflexFilter(10.0) - 4.5) < 1e-12);
    assert(std::abs(node.applyLocalReflexFilter(-10.0) - (-2.5)) < 1e-12);
}

void test_topology_and_duplicate_protection() {
    SpatialAdaptiveMesh mesh;
    mesh.addNode(0, {0.0, 0.0, 0.0}, 1.618);
    mesh.addNode(1, {1.0, 1.0, 1.0}, 1.618);
    mesh.addNode(2, {2.0, 2.0, 2.0}, 1.618);

    mesh.connectNodes(0, 1);
    mesh.connectNodes(1, 2);

    assert(mesh.getNodeBridgesCount(0) == 1);
    assert(mesh.getNodeBridgesCount(1) == 2);
    assert(mesh.getNodeBridgesCount(2) == 1);

    mesh.autoConnectNearbyNodes(3.0);
    assert(mesh.getNodeBridgesCount(0) == 1);

    mesh.autoConnectNearbyNodes(3.5);
    assert(mesh.getNodeBridgesCount(0) == 2);
    assert(mesh.getNodeBridgesCount(2) == 2);

    mesh.autoConnectNearbyNodes(3.5);
    assert(mesh.getNodeBridgesCount(0) == 2);
    assert(mesh.getNodeBridgesCount(1) == 2);
    assert(mesh.getNodeBridgesCount(2) == 2);
}

void test_stability_and_shock() {
    SpatialAdaptiveMesh mesh;

    mesh.addNode(0, {0.0, 0.0, 0.0}, 1.618);
    mesh.addNode(1, {1.0, 1.0, 1.0}, 1.618);
    mesh.addNode(2, {2.0, 2.0, 2.0}, 1.618);

    mesh.connectNodes(0, 1);
    mesh.connectNodes(1, 2);
    mesh.autoConnectNearbyNodes(3.5);

    mesh.injectExternalShock(0, 5.0);
    assert(std::abs(mesh.getNodeState(0) - 5.118) < 1e-4);

    mesh.simulationStepAsync();

    assert(mesh.getNodeHealth(0) < 1.0);
    for (size_t i = 0; i < 3; ++i) {
        assert(std::isfinite(mesh.getNodeState(i)));
        assert(mesh.getNodeHealth(i) >= 0.0);
        assert(mesh.getNodeHealth(i) <= 1.0);
    }

    mesh.injectExternalShock(2, 25.0);
    mesh.simulationStepAsync();
    mesh.pruneIsolatedBridges(0.05);

    for (size_t i = 0; i < 3; ++i) {
        assert(std::isfinite(mesh.getNodeState(i)));
        assert(mesh.getNodeHealth(i) >= 0.0);
        assert(mesh.getNodeHealth(i) <= 1.0);
    }
}

int main() {
    test_vector_geometry();
    test_identity_and_meta_evaluator();
    test_bridge_state_machine();
    test_local_reflex_filter();
    test_topology_and_duplicate_protection();
    test_stability_and_shock();

    std::cout << "Adaptive Mesh invariant tests passed." << std::endl;
    return 0;
}
