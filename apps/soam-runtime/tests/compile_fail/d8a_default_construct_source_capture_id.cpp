#include "source_capture_id.hpp"

int main() {
    AdaptiveMesh::SourceCaptureId id{};
    return static_cast<int>(id.bytes()[0]);
}
