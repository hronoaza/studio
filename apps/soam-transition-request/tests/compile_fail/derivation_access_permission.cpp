#include "detail/production_transition_request_internal.hpp"

using Access =
    AdaptiveMesh::detail::ProductionTransitionRequestDerivationAccess;

void misuse(AdaptiveMesh::ProductionTransitionRequestBinding binding) {
    auto evidence = Access::permission(binding, true);
    (void)evidence;
}

int main() { return 0; }
