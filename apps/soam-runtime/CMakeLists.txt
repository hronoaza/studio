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
    src/production_provenance_envelope.cpp)
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
target_compile_definitions(soam_runtime PRIVATE
    SOAM_D8B_IMPLEMENTATION_REVISION_SHA="\"${SOAM_D8B_IMPLEMENTATION_REVISION_SHA}\"")

target_include_directories(soam_runtime PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

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
        provenance_envelope_tests)
    add_executable(${test_target} tests/${test_target}.cpp)
    target_link_libraries(${test_target} PRIVATE soam_runtime)
    add_test(NAME run_${test_target} COMMAND ${test_target})
endforeach()

target_include_directories(source_snapshot_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(provenance_envelope_tests PRIVATE
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
