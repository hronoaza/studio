cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(SOAMRuntime VERSION 2.0.0 LANGUAGES CXX)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOAM_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

add_library(soam_runtime STATIC
    src/system_architecture.cpp
    src/production_transition_evaluator.cpp)
add_library(AdaptiveMesh::soam_runtime ALIAS soam_runtime)

target_include_directories(soam_runtime PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(soam_runtime PUBLIC Threads::Threads)
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
        source_snapshot_tests)
    add_executable(${test_target} tests/${test_target}.cpp)
    target_link_libraries(${test_target} PRIVATE soam_runtime)
    add_test(NAME run_${test_target} COMMAND ${test_target})
endforeach()


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
