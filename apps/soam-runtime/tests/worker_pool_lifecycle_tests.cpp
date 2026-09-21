#include "system_architecture.hpp"

#include <cmath>
#include <cstddef>
#include <iostream>
#include <stdexcept>

using namespace AdaptiveMesh;

namespace {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void populateLinearMesh(SpatialAdaptiveMesh& mesh, std::size_t nodeCount) {
    for (std::size_t nodeId = 0; nodeId < nodeCount; ++nodeId) {
        mesh.addNode(nodeId, {static_cast<double>(nodeId), 0.0, 0.0}, 0.0);
    }
    for (std::size_t nodeId = 1; nodeId < nodeCount; ++nodeId) {
        mesh.connectNodes(
            static_cast<int>(nodeId - 1),
            static_cast<int>(nodeId));
    }
}

} // namespace

int main() {
    constexpr std::size_t lifecycleIterations = 64;

    for (std::size_t iteration = 0; iteration < lifecycleIterations; ++iteration) {
        for (std::size_t workerCount :
             {std::size_t{1}, std::size_t{2}, std::size_t{4}}) {
            SpatialAdaptiveMesh mesh(workerCount);
            populateLinearMesh(mesh, 8);
            mesh.injectExternalShock(0, 2.0);
            for (int step = 0; step < 5; ++step) {
                mesh.simulationStep();
            }
            require(std::isfinite(mesh.getNodeState(0)),
                    "fixed worker pool must produce a finite state");
        }

        SpatialAdaptiveMesh expandingMesh(4);
        expandingMesh.addNode(0, {0.0, 0.0, 0.0}, 0.0);
        expandingMesh.simulationStep();

        for (std::size_t nodeId = 1; nodeId < 8; ++nodeId) {
            expandingMesh.addNode(
                nodeId,
                {static_cast<double>(nodeId), 0.0, 0.0},
                0.0);
        }
        for (int nodeId = 1; nodeId < 8; ++nodeId) {
            expandingMesh.connectNodes(nodeId - 1, nodeId);
        }
        expandingMesh.simulationStep();
    }

    std::cout << "SOAM worker pool lifecycle tests passed.\n";
    return 0;
}
