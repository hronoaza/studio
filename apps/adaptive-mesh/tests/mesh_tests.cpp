#include <cassert>
#include <iostream>
#include "system_architecture.hpp"

void test_stability_and_shock() {
    using namespace AdaptiveMesh;
    SpatialAdaptiveMesh mesh;

    mesh.addNode(0, {0.0, 0.0, 0.0}, 1.618);
    mesh.addNode(1, {1.0, 1.0, 1.0}, 1.618);
    mesh.addNode(2, {2.0, 2.0, 2.0}, 1.618);

    mesh.connectNodes(0, 1);
    mesh.connectNodes(1, 2);

    assert(std::abs(mesh.getNodeState(0) - 1.618) < 1e-6);

    mesh.autoConnectNearbyNodes(3.0); // Historical test assumption expects 0 and 2 to connect.
    assert(mesh.getNodeBridgesCount(0) == 2);

    mesh.injectExternalShock(0, 5.0);
    assert(std::abs(mesh.getNodeState(0) - 5.118) < 1e-4);

    mesh.simulationStepAsync();

    assert(mesh.getNodeHealth(0) < 1.0);

    mesh.injectExternalShock(2, 25.0);
    mesh.simulationStepAsync();
    mesh.pruneIsolatedBridges(0.05);

    std::cout << "All SOAM RC Unit Tests Passed successfully!" << std::endl;
}

int main() {
    test_stability_and_shock();
    return 0;
}
