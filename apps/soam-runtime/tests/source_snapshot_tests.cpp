#include "production_relationship_source_snapshot.hpp"
#include "system_architecture.hpp"
#include "detail/source_capture_id_internal.hpp"

#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

using namespace AdaptiveMesh;

[[noreturn]] void fail() noexcept { std::abort(); }
void require(bool condition) noexcept { if (!condition) fail(); }
void requireNear(double actual, double expected, double epsilon = 1e-12) noexcept {
    if (std::abs(actual - expected) > epsilon) fail();
}

bool isAllZero(const SourceCaptureId::Bytes& bytes) noexcept {
    for (const auto value : bytes) {
        if (value != 0U) return false;
    }
    return true;
}

static_assert(!std::is_default_constructible_v<SourceCaptureId>);
static_assert(!std::is_constructible_v<
    SourceCaptureId,
    SourceCaptureId::Bytes>);
static_assert(!std::is_default_constructible_v<
    ProductionRelationshipSourceSnapshot>);
static_assert(!std::is_constructible_v<
    ProductionRelationshipSourceSnapshot,
    SourceCaptureId,
    std::size_t,
    std::size_t,
    std::uint64_t,
    std::uint64_t,
    double,
    double,
    double,
    BridgeStatus,
    double,
    double,
    double,
    double>);

namespace {

std::atomic<int> scriptedCall{0};
SourceCaptureId::Bytes scriptedFirst{};
SourceCaptureId::Bytes scriptedSecond{};

bool failFill(SourceCaptureId::Bytes&) noexcept {
    return false;
}

bool zeroFill(SourceCaptureId::Bytes& out) noexcept {
    out = {};
    ++scriptedCall;
    return true;
}

bool zeroThenFixedFill(SourceCaptureId::Bytes& out) noexcept {
    const int call = scriptedCall.fetch_add(1);
    if (call == 0) {
        out = {};
    } else {
        out = scriptedFirst;
    }
    return true;
}

bool duplicateThenSecondFill(SourceCaptureId::Bytes& out) noexcept {
    const int call = scriptedCall.fetch_add(1);
    out = call <= 1 ? scriptedFirst : scriptedSecond;
    return true;
}

} // namespace

int main() {
    detail::resetSourceCaptureIdGeneratorForTesting();

    SpatialAdaptiveMesh mesh(2);
    mesh.addNode(0, {0.0, 0.0, 0.0}, 1.0);
    mesh.addNode(1, {1.0, 0.0, 0.0}, 1.0);

    require(!mesh.captureProductionRelationshipSourceSnapshot({0, 1}).has_value());
    require(!mesh.captureProductionRelationshipSourceSnapshot({0, 0}).has_value());
    require(!mesh.captureProductionRelationshipSourceSnapshot({0, 99}).has_value());

    mesh.connectNodes(0, 1);

    const auto first =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(first.has_value());

    require(!isAllZero(first->sourceCaptureId().bytes()));
    require(first->sourceCaptureId().bytes().size() == 16);
    require(first->sourceNodeId() == 0);
    require(first->targetNodeId() == 1);
    require(first->relationshipGeneration() > 0);
    require(first->stateVersion() > 0);
    requireNear(first->distance(), 1.0);
    requireNear(first->orientationWeight(), 0.5);
    requireNear(first->capacity(), 1.0);
    require(first->bridgeStatus() == BridgeStatus::NORMAL);
    requireNear(first->sourceState(), 1.0);
    requireNear(first->targetState(), 1.0);
    requireNear(first->sourceHealth(), 1.0);
    requireNear(first->targetHealth(), 1.0);

    const auto firstCopy = *first;
    require(firstCopy.sourceCaptureId() == first->sourceCaptureId());

    auto movedCopy = firstCopy;
    const auto firstIdBeforeMove = movedCopy.sourceCaptureId().bytes();
    auto movedSnapshot = std::move(movedCopy);
    require(movedSnapshot.sourceCaptureId().bytes() == firstIdBeforeMove);

    const auto same =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(same.has_value());
    require(same->relationshipGeneration() == first->relationshipGeneration());
    require(same->stateVersion() == first->stateVersion());
    require(same->sourceCaptureId() != first->sourceCaptureId());

    const auto reverse =
        mesh.captureProductionRelationshipSourceSnapshot({1, 0});
    require(reverse.has_value());
    require(reverse->relationshipGeneration() == first->relationshipGeneration());
    require(reverse->sourceCaptureId() != first->sourceCaptureId());
    requireNear(reverse->distance(), first->distance());
    requireNear(reverse->orientationWeight(), 0.5);

    mesh.injectExternalShock(0, 1.0);

    const auto afterShock =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(afterShock.has_value());
    require(afterShock->relationshipGeneration() == first->relationshipGeneration());
    require(afterShock->stateVersion() > first->stateVersion());
    require(afterShock->sourceState() != first->sourceState());
    require(afterShock->sourceCaptureId() != first->sourceCaptureId());

    mesh.simulationStep();

    const auto afterStep =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(afterStep.has_value());
    require(afterStep->relationshipGeneration() == first->relationshipGeneration());
    require(afterStep->stateVersion() > afterShock->stateVersion());
    require(afterStep->sourceCaptureId() != afterShock->sourceCaptureId());

    // Concurrent successful recaptures are distinct events.
    constexpr std::size_t concurrentCount = 8;
    std::array<SourceCaptureId::Bytes, concurrentCount> concurrentIds{};
    std::array<bool, concurrentCount> concurrentSuccess{};
    std::vector<std::thread> threads;
    threads.reserve(concurrentCount);

    for (std::size_t i = 0; i < concurrentCount; ++i) {
        threads.emplace_back([&mesh, &concurrentIds, &concurrentSuccess, i] {
            const auto snapshot =
                mesh.captureProductionRelationshipSourceSnapshot({0, 1});
            if (snapshot.has_value()) {
                concurrentIds[i] = snapshot->sourceCaptureId().bytes();
                concurrentSuccess[i] = true;
            }
        });
    }
    for (auto& thread : threads) thread.join();

    for (std::size_t i = 0; i < concurrentCount; ++i) {
        require(concurrentSuccess[i]);
        require(!isAllZero(concurrentIds[i]));
        for (std::size_t j = 0; j < i; ++j) {
            require(concurrentIds[i] != concurrentIds[j]);
        }
    }

    // Hard random-source failure fails closed and leaves live state unchanged.
    const double stateBeforeFailure = mesh.getNodeState(0);
    const double healthBeforeFailure = mesh.getNodeHealth(0);
    detail::setSourceCaptureIdFillFunctionForTesting(&failFill);
    require(!mesh.captureProductionRelationshipSourceSnapshot({0, 1}).has_value());
    requireNear(mesh.getNodeState(0), stateBeforeFailure);
    requireNear(mesh.getNodeHealth(0), healthBeforeFailure);

    // All-zero first candidate is retried, then a fixed valid ID is published.
    detail::resetSourceCaptureIdGeneratorForTesting();
    scriptedCall.store(0);
    scriptedFirst.fill(0x31);
    detail::setSourceCaptureIdFillFunctionForTesting(&zeroThenFixedFill);
    const auto afterZeroRetry =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(afterZeroRetry.has_value());
    require(afterZeroRetry->sourceCaptureId().bytes() == scriptedFirst);
    require(scriptedCall.load() == 2);

    // A locally detected duplicate is retried.
    detail::resetSourceCaptureIdGeneratorForTesting();
    scriptedCall.store(0);
    scriptedFirst.fill(0x41);
    scriptedSecond.fill(0x42);
    detail::setSourceCaptureIdFillFunctionForTesting(&duplicateThenSecondFill);

    const auto duplicateSeed =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(duplicateSeed.has_value());
    require(duplicateSeed->sourceCaptureId().bytes() == scriptedFirst);

    const auto duplicateRetry =
        mesh.captureProductionRelationshipSourceSnapshot({0, 1});
    require(duplicateRetry.has_value());
    require(duplicateRetry->sourceCaptureId().bytes() == scriptedSecond);
    require(scriptedCall.load() == 3);

    // Eight invalid candidates exhaust the finite retry budget.
    detail::resetSourceCaptureIdGeneratorForTesting();
    scriptedCall.store(0);
    detail::setSourceCaptureIdFillFunctionForTesting(&zeroFill);
    require(!mesh.captureProductionRelationshipSourceSnapshot({0, 1}).has_value());
    require(scriptedCall.load() == 8);

    detail::resetSourceCaptureIdGeneratorForTesting();
    return 0;
}
