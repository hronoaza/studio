#!/usr/bin/env bash
set -euo pipefail

pattern='ProductionTransitionConstructionAccess|PermissionPrerequisiteEvidence|InvariantPrerequisiteEvidence|ResiliencePrerequisiteEvidence|FreshnessPrerequisiteEvidence|RevalidationPrerequisiteEvidence|ProductionTransitionPrerequisiteSet|ProductionTransitionEligibilityDecision|ProductionTransitionEligibilityEvaluator|ProductionPermissionC1Adapter'

scan_root() {
  local root="$1"
  local hits
  hits="$(
    find "$root/include" "$root/src" -type f \( -name '*.hpp' -o -name '*.cpp' \) -print0 |
      xargs -0 grep -En "$pattern" || true
  )"
  if [[ -n "$hits" ]]; then
    echo "ERROR: forbidden C1/eligibility symbol used by permission production code"
    echo "$hits"
    return 1
  fi
}

if [[ "${1:-}" == "--self-test" ]]; then
  tmp="$(mktemp -d)"
  trap 'rm -rf "$tmp"' EXIT
  mkdir -p "$tmp/include" "$tmp/src"
  printf '%s\n' 'struct PermissionPrerequisiteEvidence;' > "$tmp/include/bad.hpp"
  if scan_root "$tmp"; then
    echo "ERROR: forbidden-symbol gate failed to detect positive control"
    exit 1
  fi
  echo "forbidden-symbol gate positive control correctly rejected"
  exit 0
fi

scan_root "$1"
echo "permission production surface contains no forbidden C1/eligibility symbol use"
