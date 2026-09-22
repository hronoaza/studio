#include "source_capture_id.hpp"

int main() {
    AdaptiveMesh::SourceCaptureId::Bytes bytes{};
    bytes[0] = 1;
    AdaptiveMesh::SourceCaptureId id{bytes};
    return static_cast<int>(id.bytes()[0]);
}
