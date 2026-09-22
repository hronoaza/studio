#include "production_permission_attestation.hpp"
#include <vector>
int main(){ std::vector<unsigned char> a,b,c; auto x=AdaptiveMesh::verifyEd25519(a,b,c); (void)x; }
