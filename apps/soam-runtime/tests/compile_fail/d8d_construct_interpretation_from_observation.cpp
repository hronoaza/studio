#include "versioned_interpretation.hpp"
int main() {
    AdaptiveMesh::InteractionObservation observation{0.5};
    AdaptiveMesh::BridgeConfidence confidence{1.0};
    AdaptiveMesh::VersionedProductionInterpretation value{
        observation, confidence};
    return 0;
}
