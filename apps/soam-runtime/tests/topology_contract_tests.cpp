#include "system_architecture.hpp"

#include <iostream>
#include <stdexcept>

using namespace AdaptiveMesh;

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
}

int main() {
    SpatialAdaptiveMesh mesh(1);
    for (std::size_t i = 0; i < 4; ++i) {
        mesh.addNode(i, {static_cast<double>(i), 0.0, 0.0}, 0.0);
    }

    mesh.connectNodes(0, 1);

    bool batchRejected = false;
    try {
        mesh.connectNodePairs({{1, 2}, {2, 2}});
    } catch (const std::invalid_argument&) {
        batchRejected = true;
    }

    require(batchRejected, "invalid batch must be rejected");
    require(mesh.getNodeBridgesCount(1) == 1,
            "failed batch must not publish a partial connection");
    require(mesh.getNodeBridgesCount(2) == 0,
            "failed batch must leave untouched nodes unchanged");

    mesh.autoConnectNearbyNodes(1.1);
    require(mesh.getNodeBridgesCount(1) == 2,
            "automatic topology connection must add eligible neighbor");

    mesh.simulationStep();
    std::cout << "SOAM topology contract tests passed.\n";
    return 0;
}
