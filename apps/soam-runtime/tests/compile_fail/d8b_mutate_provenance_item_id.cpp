#include "production_provenance_envelope.hpp"
void mutate(const AdaptiveMesh::ProductionProvenanceEnvelope& envelope) {
    envelope.provenanceItemId().bytes()[0]=0x42;
}
int main(){ return 0; }
