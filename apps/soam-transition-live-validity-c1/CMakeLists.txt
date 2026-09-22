cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(SOAMTransitionLiveValidityC1 VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOAM_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-live-validity"
    "${CMAKE_CURRENT_BINARY_DIR}/soam-transition-live-validity")

add_library(soam_transition_live_validity_c1 STATIC
    src/production_live_validity_c1_adapter.cpp)
add_library(
    AdaptiveMesh::soam_transition_live_validity_c1
    ALIAS soam_transition_live_validity_c1)

target_include_directories(soam_transition_live_validity_c1
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(soam_transition_live_validity_c1
    PUBLIC
        AdaptiveMesh::soam_transition_live_validity
        soam_transition_eligibility)

target_compile_features(soam_transition_live_validity_c1 PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soam_transition_live_validity_c1 PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(SOAM_WARNINGS_AS_ERRORS)
        target_compile_options(soam_transition_live_validity_c1 PRIVATE -Werror)
    endif()
endif()

enable_testing()

add_executable(
    live_validity_c1_adapter_tests
    tests/live_validity_c1_adapter_tests.cpp)
target_link_libraries(
    live_validity_c1_adapter_tests
    PRIVATE soam_transition_live_validity_c1)
target_include_directories(
    live_validity_c1_adapter_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-live-validity/src)
add_test(
    NAME run_live_validity_c1_adapter_tests
    COMMAND live_validity_c1_adapter_tests)

function(add_a2_compile_fail_test test_name source description)
    add_test(
        NAME ${test_name}
        COMMAND bash
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_compile_failure.sh
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/compile_fail/${source}
            "${description}"
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-live-validity/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-live-validity/src
            ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../soam-domain/include
            ${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition/include)
endfunction()

add_a2_compile_fail_test(
    reject_a2_default_construct_wrapper
    default_construct_wrapper.cpp
    "A2 restricted-origin wrapper default construction")
add_a2_compile_fail_test(
    reject_a2_direct_construct_freshness
    direct_construct_freshness.cpp
    "A2 caller direct freshness evidence construction")
add_a2_compile_fail_test(
    reject_a2_access_permission
    access_permission.cpp
    "A2 access cannot construct permission evidence")
add_a2_compile_fail_test(
    reject_a2_loose_record_overload
    loose_record_overload.cpp
    "A2 adapter cannot accept loose A1 records")
add_a2_compile_fail_test(
    reject_a2_eligibility_surface
    no_eligibility_api.cpp
    "A2 adapter exposes no eligibility API")
