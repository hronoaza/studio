#include "system_architecture.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>

using namespace AdaptiveMesh;

namespace {
void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
}

int main() {
    SpatialAdaptiveMesh mesh(4);
    for (std::size_t i = 0; i < 8; ++i) {
        mesh.addNode(i, {static_cast<double>(i), 0.0, 0.0}, 1.618);
    }

    mesh.connectNodePairs({
        {0, 1}, {1, 2}, {2, 3}, {3, 4},
        {4, 5}, {5, 6}, {6, 7}
    });

    require(mesh.getNodeBridgesCount(1) == 2, "batch topology must be symmetric");

    bool duplicateRejected = false;
    try {
        mesh.connectNodes(0, 1);
    } catch (const std::invalid_argument&) {
        duplicateRejected = true;
    }
    require(duplicateRejected, "duplicate direct connection must be rejected");

    bool invalidIndexRejected = false;
    try {
        mesh.connectNodes(-1, 1);
    } catch (const std::out_of_range&) {
        invalidIndexRejected = true;
    }
    require(invalidIndexRejected, "invalid node index must be rejected");

    mesh.injectExternalShock(0, 5.0);
    for (int step = 0; step < 100; ++step) {
        mesh.simulationStep();
    }

    for (std::size_t i = 0; i < 8; ++i) {
        require(std::isfinite(mesh.getNodeState(i)), "state must remain finite");
        require(mesh.getNodeHealth(i) >= 0.0 && mesh.getNodeHealth(i) <= 1.0,
                "health must remain in [0, 1]");
    }

    mesh.simulationStepAsync();
    std::cout << "SOAM runtime tests passed.\n";
    return 0;
}
