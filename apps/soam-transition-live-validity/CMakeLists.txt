cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(SOAMTransitionLiveValidity VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOAM_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request"
    "${CMAKE_CURRENT_BINARY_DIR}/soam-transition-request")

add_library(soam_transition_live_validity STATIC
    src/production_transition_live_validity.cpp)
add_library(
    AdaptiveMesh::soam_transition_live_validity
    ALIAS soam_transition_live_validity)

target_include_directories(soam_transition_live_validity
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(soam_transition_live_validity
    PUBLIC
        AdaptiveMesh::soam_runtime
        AdaptiveMesh::soam_transition_request)

if(WIN32)
    target_link_libraries(soam_transition_live_validity PRIVATE bcrypt)
endif()

target_compile_features(soam_transition_live_validity PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soam_transition_live_validity PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(SOAM_WARNINGS_AS_ERRORS)
        target_compile_options(soam_transition_live_validity PRIVATE -Werror)
    endif()
endif()

enable_testing()

add_executable(
    transition_live_validity_tests
    tests/transition_live_validity_tests.cpp)
target_link_libraries(
    transition_live_validity_tests
    PRIVATE soam_transition_live_validity)
target_include_directories(
    transition_live_validity_tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(
    NAME run_transition_live_validity_tests
    COMMAND transition_live_validity_tests)


add_test(
    NAME reject_live_validity_default_parent_id
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/default_construct_parent_id.cpp
        "live validity parent decision ID default construction"
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)


add_test(
    NAME reject_live_validity_freshness_id_bytes
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/construct_freshness_id_from_bytes.cpp
        "freshness decision ID raw-byte construction"
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)


add_test(
    NAME reject_live_validity_default_freshness_record
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/default_construct_freshness_record.cpp
        "freshness record direct construction"
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)


add_test(
    NAME reject_live_validity_default_revalidation_record
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/default_construct_revalidation_record.cpp
        "revalidation record direct construction"
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)


add_test(
    NAME reject_live_validity_default_evidence
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/default_construct_evidence.cpp
        "live validity evidence direct construction"
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)


add_test(
    NAME reject_live_validity_forged_snapshot
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/forge_snapshot.cpp
        "forged runtime snapshot construction"
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)


add_test(
    NAME reject_live_validity_eligibility_surface
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/no_eligibility_api.cpp
        "live validity eligibility surface"
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)


add_test(
    NAME reject_live_validity_permission_surface
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/no_permission_api.cpp
        "live validity permission surface"
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)


add_test(
    NAME reject_live_validity_mutation_surface
    COMMAND bash
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/no_mutation_api.cpp
        "live snapshot mutation surface"
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)
