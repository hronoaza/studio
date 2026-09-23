#include "production_transition_permission.hpp"
#include <cstdlib>

int main() {
    if (AdaptiveMesh::ProductionPermissionPolicyProvider::createCurrent().has_value()) {
        std::abort();
    }
    return 0;
}
