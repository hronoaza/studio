#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>
#include "system_architecture.hpp"

using namespace AdaptiveMesh;

int main() {
    constexpr std::size_t nodeCount = 32;
    constexpr int steps = 100;

    SpatialAdaptiveMesh mesh;

    for (std::size_t i = 0; i < nodeCount; ++i) {
        const double x = static_cast<double>(i % 8);
        const double y = static_cast<double>((i / 8) % 4);
        const double z = static_cast<double>(i % 3) * 0.25;
        mesh.addNode(i, {x, y, z}, 1.618);
    }

    mesh.autoConnectNearbyNodes(1.6);

    for (std::size_t i = 0; i < nodeCount; ++i) {
        assert(mesh.getNodeBridgesCount(i) > 0);
    }

    for (int step = 0; step < steps; ++step) {
        if (step % 7 == 0) {
            const int target = step % static_cast<int>(nodeCount);
            mesh.injectExternalShock(target, (step % 2 == 0) ? 5.0 : -5.0);
        }

        mesh.simulationStepAsync();

        for (std::size_t i = 0; i < nodeCount; ++i) {
            const double state = mesh.getNodeState(i);
            const double health = mesh.getNodeHealth(i);
            assert(std::isfinite(state));
            assert(std::isfinite(health));
            assert(health >= 0.0);
            assert(health <= 1.0);
        }
    }

    mesh.pruneIsolatedBridges(0.05);

    for (std::size_t i = 0; i < nodeCount; ++i) {
        assert(std::isfinite(mesh.getNodeState(i)));
        assert(mesh.getNodeHealth(i) >= 0.0);
        assert(mesh.getNodeHealth(i) <= 1.0);
    }

    std::cout << "Adaptive Mesh stress test passed." << std::endl;
    return 0;
}
