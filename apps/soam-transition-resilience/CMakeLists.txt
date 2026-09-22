cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(SOAMTransitionResilience VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOAM_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request"
    "${CMAKE_CURRENT_BINARY_DIR}/soam-transition-request")

add_library(soam_transition_resilience STATIC
    src/production_transition_resilience.cpp)
add_library(
    AdaptiveMesh::soam_transition_resilience
    ALIAS soam_transition_resilience)

file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/include/production_transition_resilience.hpp"
    SOAM_TRANSITION_RESILIENCE_PUBLIC_SHA)
file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/production_transition_resilience_internal.hpp"
    SOAM_TRANSITION_RESILIENCE_INTERNAL_SHA)
file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/src/production_transition_resilience.cpp"
    SOAM_TRANSITION_RESILIENCE_SOURCE_SHA)
set(SOAM_TRANSITION_RESILIENCE_IMPLEMENTATION_MANIFEST
    "SOAM:TRANSITION-RESILIENCE:IMPLEMENTATION-MANIFEST:V1\npublic=${SOAM_TRANSITION_RESILIENCE_PUBLIC_SHA}\ninternal=${SOAM_TRANSITION_RESILIENCE_INTERNAL_SHA}\nsource=${SOAM_TRANSITION_RESILIENCE_SOURCE_SHA}\n")
string(SHA256 SOAM_TRANSITION_RESILIENCE_IMPLEMENTATION_REVISION_SHA
    "${SOAM_TRANSITION_RESILIENCE_IMPLEMENTATION_MANIFEST}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/transition_resilience_implementation_revision.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/transition_resilience_implementation_revision.hpp"
    @ONLY)

target_include_directories(soam_transition_resilience
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/generated)

target_link_libraries(soam_transition_resilience
    PUBLIC
        AdaptiveMesh::soam_runtime
        AdaptiveMesh::soam_transition_request)

if(WIN32)
    target_link_libraries(soam_transition_resilience PRIVATE bcrypt)
endif()

target_compile_features(soam_transition_resilience PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soam_transition_resilience PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(SOAM_WARNINGS_AS_ERRORS)
        target_compile_options(soam_transition_resilience PRIVATE -Werror)
    endif()
endif()

enable_testing()
