#include "detail/source_capture_id_internal.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#elif defined(__linux__)
#include <cerrno>
#include <sys/random.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <cstdlib>
#endif

namespace AdaptiveMesh::detail {
namespace {

constexpr std::size_t kMaxAttempts = 8;
constexpr std::size_t kRecentIdCount = 64;

std::atomic<SourceCaptureIdFillFunction> testFillFunction{nullptr};
std::atomic_flag recentIdsLock = ATOMIC_FLAG_INIT;
std::array<SourceCaptureId::Bytes, kRecentIdCount> recentIds{};
std::size_t recentIdsSize = 0;
std::size_t nextRecentId = 0;

class RecentIdsGuard final {
public:
    RecentIdsGuard() noexcept {
        while (recentIdsLock.test_and_set(std::memory_order_acquire)) {
        }
    }

    ~RecentIdsGuard() {
        recentIdsLock.clear(std::memory_order_release);
    }

    RecentIdsGuard(const RecentIdsGuard&) = delete;
    RecentIdsGuard& operator=(const RecentIdsGuard&) = delete;
};

[[nodiscard]] bool isAllZero(
    const SourceCaptureId::Bytes& bytes) noexcept
{
    for (const auto value : bytes) {
        if (value != 0U) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool reserveIfNotRecent(
    const SourceCaptureId::Bytes& bytes) noexcept
{
    RecentIdsGuard guard;

    for (std::size_t index = 0; index < recentIdsSize; ++index) {
        if (recentIds[index] == bytes) {
            return false;
        }
    }

    if (recentIdsSize < kRecentIdCount) {
        recentIds[recentIdsSize++] = bytes;
    } else {
        recentIds[nextRecentId] = bytes;
        nextRecentId = (nextRecentId + 1U) % kRecentIdCount;
    }

    return true;
}

[[nodiscard]] bool fillFromOperatingSystem(
    SourceCaptureId::Bytes& out) noexcept
{
#if defined(_WIN32)
    const NTSTATUS status = BCryptGenRandom(
        nullptr,
        reinterpret_cast<PUCHAR>(out.data()),
        static_cast<ULONG>(out.size()),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return status >= 0;
#elif defined(__linux__)
    std::size_t offset = 0;
    while (offset < out.size()) {
        const auto result = ::getrandom(
            out.data() + offset,
            out.size() - offset,
            0);
        if (result > 0) {
            offset += static_cast<std::size_t>(result);
            continue;
        }
        if (result < 0 && errno == EINTR) {
            continue;
        }
        return false;
    }
    return true;
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    ::arc4random_buf(out.data(), out.size());
    return true;
#else
    static_cast<void>(out);
    return false;
#endif
}

[[nodiscard]] bool fillCandidate(
    SourceCaptureId::Bytes& out) noexcept
{
    if (const auto overrideFill =
            testFillFunction.load(std::memory_order_acquire)) {
        return overrideFill(out);
    }
    return fillFromOperatingSystem(out);
}

} // namespace

std::optional<SourceCaptureId::Bytes>
tryGenerateSourceCaptureIdBytes() noexcept
{
    for (std::size_t attempt = 0; attempt < kMaxAttempts; ++attempt) {
        SourceCaptureId::Bytes candidate{};

        if (!fillCandidate(candidate)) {
            return std::nullopt;
        }

        if (isAllZero(candidate)) {
            continue;
        }

        if (!reserveIfNotRecent(candidate)) {
            continue;
        }

        return candidate;
    }

    return std::nullopt;
}

void setSourceCaptureIdFillFunctionForTesting(
    SourceCaptureIdFillFunction fillFunction) noexcept
{
    testFillFunction.store(fillFunction, std::memory_order_release);
}

void resetSourceCaptureIdGeneratorForTesting() noexcept
{
    testFillFunction.store(nullptr, std::memory_order_release);

    RecentIdsGuard guard;
    recentIds = {};
    recentIdsSize = 0;
    nextRecentId = 0;
}

} // namespace AdaptiveMesh::detail
