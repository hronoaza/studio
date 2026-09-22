cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(SOAMRuntime VERSION 2.0.0 LANGUAGES CXX)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOAM_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

add_library(soam_runtime STATIC
    src/system_architecture.cpp
    src/production_transition_evaluator.cpp
    src/source_capture_id.cpp
    src/production_provenance_envelope.cpp
    src/retained_source_evidence.cpp
    src/provenance_admissibility.cpp
    src/versioned_interpretation.cpp
    src/policy_persistence.cpp
    src/policy_persistence_stream.cpp)
add_library(AdaptiveMesh::soam_runtime ALIAS soam_runtime)

# D8B implementation revision is a reproducible digest of the implementation
# source surface, independent of build directory and documentation-only commits.
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/include/production_provenance_envelope.hpp"
    SOAM_D8B_PUBLIC_SHA)
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/provenance_envelope_internal.hpp"
    SOAM_D8B_INTERNAL_SHA)
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/src/production_provenance_envelope.cpp"
    SOAM_D8B_SOURCE_SHA)
set(SOAM_D8B_IMPLEMENTATION_MANIFEST
    "SOAM:D8B:IMPLEMENTATION-MANIFEST:V1\npublic=${SOAM_D8B_PUBLIC_SHA}\ninternal=${SOAM_D8B_INTERNAL_SHA}\nsource=${SOAM_D8B_SOURCE_SHA}\n")
string(SHA256 SOAM_D8B_IMPLEMENTATION_REVISION_SHA
    "${SOAM_D8B_IMPLEMENTATION_MANIFEST}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/d8b_implementation_revision.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/d8b_implementation_revision.hpp"
    @ONLY)

# D8D implementation revision is a reproducible digest of the versioned
# interpretation implementation surface.
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/include/versioned_interpretation.hpp"
    SOAM_D8D_PUBLIC_SHA)
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/versioned_interpretation_internal.hpp"
    SOAM_D8D_INTERNAL_SHA)
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/src/versioned_interpretation.cpp"
    SOAM_D8D_SOURCE_SHA)
set(SOAM_D8D_IMPLEMENTATION_MANIFEST
    "SOAM:D8D:IMPLEMENTATION-MANIFEST:V1\npublic=${SOAM_D8D_PUBLIC_SHA}\ninternal=${SOAM_D8D_INTERNAL_SHA}\nsource=${SOAM_D8D_SOURCE_SHA}\n")
string(SHA256 SOAM_D8D_IMPLEMENTATION_REVISION_SHA
    "${SOAM_D8D_IMPLEMENTATION_MANIFEST}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/d8d_implementation_revision.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/d8d_implementation_revision.hpp"
    @ONLY)

# D9 implementation revision is a reproducible digest of the policy-evidence
# and persistence-policy implementation surface.
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/include/policy_persistence.hpp"
    SOAM_D9_PUBLIC_SHA)
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/policy_persistence_internal.hpp"
    SOAM_D9_INTERNAL_SHA)
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/policy_persistence_binding.hpp"
    SOAM_D9_BINDING_SHA)
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/src/policy_persistence.cpp"
    SOAM_D9_SOURCE_SHA)
file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/src/policy_persistence_stream.cpp"
    SOAM_D9_STREAM_SOURCE_SHA)
set(SOAM_D9_IMPLEMENTATION_MANIFEST
    "SOAM:D9:IMPLEMENTATION-MANIFEST:V1\npublic=${SOAM_D9_PUBLIC_SHA}\ninternal=${SOAM_D9_INTERNAL_SHA}\nbinding=${SOAM_D9_BINDING_SHA}\nsource=${SOAM_D9_SOURCE_SHA}\nstream=${SOAM_D9_STREAM_SOURCE_SHA}\n")
string(SHA256 SOAM_D9_IMPLEMENTATION_REVISION_SHA
    "${SOAM_D9_IMPLEMENTATION_MANIFEST}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/d9_implementation_revision.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/d9_implementation_revision.hpp"
    @ONLY)

target_include_directories(soam_runtime
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include>
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")

target_link_libraries(soam_runtime PUBLIC Threads::Threads)
if(WIN32)
    target_link_libraries(soam_runtime PRIVATE bcrypt)
endif()
target_compile_features(soam_runtime PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soam_runtime PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(SOAM_WARNINGS_AS_ERRORS)
        target_compile_options(soam_runtime PRIVATE -Werror)
    endif()
endif()

enable_testing()

foreach(test_target
        runtime_tests
        worker_pool_lifecycle_tests
        topology_contract_tests
        live_evaluator_tests
        d7_provenance_gate_tests
        source_snapshot_tests
        provenance_envelope_tests
        provenance_admissibility_tests
        versioned_interpretation_tests
        policy_persistence_tests
        policy_persistence_stream_tests)
    add_executable(${test_target} tests/${test_target}.cpp)
    target_link_libraries(${test_target} PRIVATE soam_runtime)
    add_test(NAME run_${test_target} COMMAND ${test_target})
endforeach()

target_include_directories(source_snapshot_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(provenance_envelope_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(provenance_admissibility_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(versioned_interpretation_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(policy_persistence_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(policy_persistence_stream_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_test(
    NAME reject_d7_public_provenance_injection
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d7_public_provenance_injection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D7 public provenance injection misuse")


add_test(
    NAME reject_d8a_forged_source_snapshot
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8a_forge_source_snapshot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8A forged source snapshot misuse")


add_test(
    NAME reject_d8a_default_construct_source_capture_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8a_default_construct_source_capture_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8A SourceCaptureId default construction misuse")

add_test(
    NAME reject_d8a_construct_source_capture_id_from_bytes
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8a_construct_source_capture_id_from_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8A SourceCaptureId arbitrary-byte construction misuse")

add_test(
    NAME reject_d8a_mutate_source_capture_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8a_mutate_source_capture_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8A SourceCaptureId mutation misuse")


add_test(
    NAME reject_d8b_default_construct_provenance_item_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8b_default_construct_provenance_item_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8B ProvenanceItemId default construction misuse")

add_test(
    NAME reject_d8b_construct_provenance_item_id_from_bytes
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8b_construct_provenance_item_id_from_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8B ProvenanceItemId arbitrary-byte construction misuse")

add_test(
    NAME reject_d8b_default_construct_provenance_envelope
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8b_default_construct_provenance_envelope.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8B provenance envelope default construction misuse")

add_test(
    NAME reject_d8b_construct_canonical_digest_from_bytes
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8b_construct_canonical_digest_from_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8B CanonicalDigest arbitrary-byte construction misuse")

add_test(
    NAME reject_d8b_mutate_provenance_item_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8b_mutate_provenance_item_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8B provenance item identity mutation misuse")


add_test(
    NAME reject_d8c_default_construct_policy_snapshot_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8c_default_construct_policy_snapshot_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8C PolicySnapshotId default construction misuse")

add_test(
    NAME reject_d8c_construct_policy_snapshot_id_from_bytes
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8c_construct_policy_snapshot_id_from_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8C PolicySnapshotId arbitrary-byte construction misuse")

add_test(
    NAME reject_d8c_default_construct_decision_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8c_default_construct_decision_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8C AdmissibilityDecisionId default construction misuse")

add_test(
    NAME reject_d8c_construct_decision_id_from_bytes
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8c_construct_decision_id_from_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8C AdmissibilityDecisionId arbitrary-byte construction misuse")

add_test(
    NAME reject_d8c_default_construct_admissible_provenance
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8c_default_construct_admissible_provenance.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8C admissible provenance default construction misuse")

add_test(
    NAME reject_d8c_construct_admissible_from_envelope
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8c_construct_admissible_from_envelope.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8C raw envelope to admissible provenance misuse")


add_test(
    NAME reject_d8d_default_construct_decision_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8d_default_construct_decision_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8D InterpretationDecisionId default construction misuse")

add_test(
    NAME reject_d8d_construct_decision_id_from_bytes
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8d_construct_decision_id_from_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8D InterpretationDecisionId arbitrary-byte construction misuse")

add_test(
    NAME reject_d8d_default_construct_interpretation
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8d_default_construct_interpretation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8D VersionedProductionInterpretation default construction misuse")

add_test(
    NAME reject_d8d_construct_interpretation_from_observation
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8d_construct_interpretation_from_observation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8D arbitrary observation/confidence trusted construction misuse")

add_test(
    NAME reject_d8d_default_construct_policy_snapshot
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d8d_default_construct_policy_snapshot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D8D interpretation policy snapshot default construction misuse")


add_test(
    NAME reject_d9_default_construct_policy_evidence_decision_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_default_construct_policy_evidence_decision_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 PolicyEvidenceDecisionId default construction misuse")

add_test(
    NAME reject_d9_construct_policy_evidence_decision_id_from_bytes
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_construct_policy_evidence_decision_id_from_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 PolicyEvidenceDecisionId arbitrary-byte construction misuse")

add_test(
    NAME reject_d9_default_construct_policy_evidence
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_default_construct_policy_evidence.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 policy evidence default construction misuse")

add_test(
    NAME reject_d9_default_construct_persistence_policy_snapshot
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_default_construct_persistence_policy_snapshot.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 persistence policy snapshot default construction misuse")


add_test(
    NAME reject_d9_default_construct_stream_instance_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_default_construct_stream_instance_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 stream instance identity default construction misuse")

add_test(
    NAME reject_d9_default_construct_observation_decision_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_default_construct_observation_decision_id.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 observation decision identity default construction misuse")

add_test(
    NAME reject_d9_default_construct_recommendation
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_default_construct_recommendation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 recommendation direct construction misuse")

add_test(
    NAME reject_d9_default_construct_persistence_rejection
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_default_construct_persistence_rejection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 persistence rejection direct construction misuse")

add_test(
    NAME reject_d9_default_construct_stream_handle
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_default_construct_stream_handle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 stream handle direct construction misuse")

add_test(
    NAME reject_d9_copy_persistence_stream
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_copy_persistence_stream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 concrete persistence stream copy misuse")


add_test(
    NAME reject_d9_construct_stream_instance_id_from_bytes
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_construct_stream_instance_id_from_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 stream instance arbitrary-byte construction misuse")

add_test(
    NAME reject_d9_construct_observation_decision_id_from_bytes
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_construct_observation_decision_id_from_bytes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 observation decision arbitrary-byte construction misuse")

add_test(
    NAME reject_d9_construct_stream_from_trusted_values
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_construct_stream_from_trusted_values.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 trusted stream direct construction misuse")

add_test(
    NAME reject_d9_move_persistence_stream
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_move_persistence_stream.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 concrete persistence stream move misuse")

add_test(
    NAME reject_d9_recommendation_to_requested_transition_direction
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_recommendation_to_requested_transition_direction.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 recommendation to transition direction misuse"
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)

add_test(
    NAME reject_d9_raw_bridge_persistence_surface
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_access_raw_bridge_persistence_from_handle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 raw BridgePersistence surface misuse")

add_test(
    NAME reject_d9_caller_selected_persistence_policy
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/d9_construct_policy_snapshot_with_caller_values.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        "D9 caller-selected trusted persistence policy misuse")
