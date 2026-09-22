#include "retained_source_evidence.hpp"
#include "system_architecture.hpp"

int main() {
    AdaptiveMesh::SourceCaptureId::Bytes bytes{};
    // Ordinary callers cannot construct SourceCaptureId or a retained record.
    AdaptiveMesh::RetainedSourceEvidenceRecord record{
        AdaptiveMesh::SourceCaptureId{bytes},
        0, 1, 1, 1,
        1.0, 1.0, 1.0,
        AdaptiveMesh::BridgeStatus::NORMAL,
        1.0, 1.0, 1.0, 1.0
    };
    return static_cast<int>(record.sourceNodeId());
}
