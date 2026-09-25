cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(SOAMTransitionPermission VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOAM_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

find_path(SODIUM_INCLUDE_DIR sodium.h REQUIRED)
find_library(SODIUM_LIBRARY NAMES sodium libsodium REQUIRED)

add_library(soam_sodium UNKNOWN IMPORTED)
set_target_properties(soam_sodium PROPERTIES
    IMPORTED_LOCATION "${SODIUM_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SODIUM_INCLUDE_DIR}")

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request"
    "${CMAKE_CURRENT_BINARY_DIR}/soam-transition-request")

file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/include/production_transition_permission.hpp"
    SOAM_TRANSITION_PERMISSION_PUBLIC_SHA)
file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/production_transition_permission_internal.hpp"
    SOAM_TRANSITION_PERMISSION_INTERNAL_SHA)
file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/src/production_transition_permission.cpp"
    SOAM_TRANSITION_PERMISSION_SOURCE_SHA)
set(SOAM_TRANSITION_PERMISSION_IMPLEMENTATION_MANIFEST
    "SOAM:TRANSITION-PERMISSION:IMPLEMENTATION-MANIFEST:V1\npublic=${SOAM_TRANSITION_PERMISSION_PUBLIC_SHA}\ninternal=${SOAM_TRANSITION_PERMISSION_INTERNAL_SHA}\nsource=${SOAM_TRANSITION_PERMISSION_SOURCE_SHA}\n")
string(SHA256 SOAM_TRANSITION_PERMISSION_IMPLEMENTATION_REVISION_SHA
    "${SOAM_TRANSITION_PERMISSION_IMPLEMENTATION_MANIFEST}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/transition_permission_implementation_revision.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/transition_permission_implementation_revision.hpp"
    @ONLY)

add_library(soam_transition_permission STATIC
    src/production_permission_attestation.cpp
    src/production_transition_permission.cpp)
add_library(
    AdaptiveMesh::soam_transition_permission
    ALIAS soam_transition_permission)

target_include_directories(soam_transition_permission
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/generated)

target_link_libraries(soam_transition_permission
    PUBLIC AdaptiveMesh::soam_transition_request
    PRIVATE soam_sodium)

if(WIN32)
    target_link_libraries(soam_transition_permission PRIVATE bcrypt)
endif()

target_compile_features(soam_transition_permission PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soam_transition_permission PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(SOAM_WARNINGS_AS_ERRORS)
        target_compile_options(soam_transition_permission PRIVATE -Werror)
    endif()
endif()

enable_testing()

add_library(soam_transition_permission_testable STATIC
    src/production_permission_attestation.cpp
    src/production_transition_permission.cpp)
target_include_directories(soam_transition_permission_testable
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(soam_transition_permission_testable
    PUBLIC AdaptiveMesh::soam_transition_request
    PRIVATE soam_sodium)
target_compile_definitions(
    soam_transition_permission_testable PRIVATE SOAM_PERMISSION_TESTING=1)
if(WIN32)
    target_link_libraries(soam_transition_permission_testable PRIVATE bcrypt)
endif()
target_compile_features(soam_transition_permission_testable PUBLIC cxx_std_20)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soam_transition_permission_testable PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(SOAM_WARNINGS_AS_ERRORS)
        target_compile_options(soam_transition_permission_testable PRIVATE -Werror)
    endif()
endif()

add_executable(
    permission_attestation_tests
    tests/permission_attestation_tests.cpp)
target_link_libraries(
    permission_attestation_tests
    PRIVATE soam_transition_permission)
target_include_directories(
    permission_attestation_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(
    NAME run_permission_attestation_tests
    COMMAND permission_attestation_tests)

add_executable(
    transition_permission_tests
    tests/transition_permission_tests.cpp)
target_link_libraries(
    transition_permission_tests
    PRIVATE soam_transition_permission_testable soam_sodium)
target_include_directories(
    transition_permission_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(
    NAME run_transition_permission_tests
    COMMAND transition_permission_tests)

add_executable(
    permission_unprovisioned_policy_tests
    tests/permission_unprovisioned_policy_tests.cpp)
target_link_libraries(
    permission_unprovisioned_policy_tests
    PRIVATE soam_transition_permission)
add_test(
    NAME run_permission_unprovisioned_policy_tests
    COMMAND permission_unprovisioned_policy_tests)

add_test(
    NAME permission_no_c1_symbol_use
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_no_c1_symbols.sh
        ${CMAKE_CURRENT_SOURCE_DIR})
add_test(
    NAME permission_no_c1_symbol_gate_self_test
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_no_c1_symbols.sh
        --self-test)

function(add_permission_compile_fail_test name source label)
    add_test(
        NAME ${name}
        COMMAND bash
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/${source}
            "${label}"
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)
endfunction()

add_permission_compile_fail_test(
    reject_permission_direct_payload
    direct_construct_payload.cpp
    "permission payload direct construction")
add_permission_compile_fail_test(
    reject_permission_direct_public_key
    direct_construct_public_key.cpp
    "Ed25519 public key direct construction")
add_permission_compile_fail_test(
    reject_permission_direct_signature
    direct_construct_signature.cpp
    "Ed25519 signature direct construction")
add_permission_compile_fail_test(
    reject_permission_signing_surface
    no_signing_api.cpp
    "production signing surface")
add_permission_compile_fail_test(
    reject_permission_dynamic_verify_surface
    no_dynamic_verify_api.cpp
    "dynamic verifier surface")
add_permission_compile_fail_test(
    reject_permission_default_decision_id
    default_construct_permission_decision_id.cpp
    "permission decision ID default construction")
add_permission_compile_fail_test(
    reject_permission_decision_id_from_bytes
    construct_permission_decision_id_from_bytes.cpp
    "permission decision ID raw-byte construction")
add_permission_compile_fail_test(
    reject_permission_default_policy_snapshot
    default_construct_permission_policy_snapshot.cpp
    "permission policy snapshot direct construction")
add_permission_compile_fail_test(
    reject_permission_default_record
    default_construct_permission_record.cpp
    "permission prerequisite record direct construction")
add_permission_compile_fail_test(
    reject_permission_c1_evidence
    construct_c1_permission.cpp
    "C1 permission evidence direct construction")
add_permission_compile_fail_test(
    reject_permission_eligibility_surface
    no_permission_eligibility_api.cpp
    "permission verifier eligibility surface")
add_permission_compile_fail_test(
    reject_permission_authority_surface
    no_permission_authority_api.cpp
    "permission verifier authority surface")
add_permission_compile_fail_test(
    reject_permission_private_key_surface
    no_permission_private_key_api.cpp
    "permission verifier private-key surface")
