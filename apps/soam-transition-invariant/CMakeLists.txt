cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(SOAMTransitionInvariant VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOAM_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition-request"
    "${CMAKE_CURRENT_BINARY_DIR}/soam-transition-request")

add_library(soam_transition_invariant STATIC
    src/production_transition_invariant.cpp)
add_library(
    AdaptiveMesh::soam_transition_invariant
    ALIAS soam_transition_invariant)

file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/include/production_transition_invariant.hpp"
    SOAM_TRANSITION_INVARIANT_PUBLIC_SHA)
file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/production_transition_invariant_internal.hpp"
    SOAM_TRANSITION_INVARIANT_INTERNAL_SHA)
file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/src/production_transition_invariant.cpp"
    SOAM_TRANSITION_INVARIANT_SOURCE_SHA)
set(SOAM_TRANSITION_INVARIANT_IMPLEMENTATION_MANIFEST
    "SOAM:TRANSITION-INVARIANT:IMPLEMENTATION-MANIFEST:V1\npublic=${SOAM_TRANSITION_INVARIANT_PUBLIC_SHA}\ninternal=${SOAM_TRANSITION_INVARIANT_INTERNAL_SHA}\nsource=${SOAM_TRANSITION_INVARIANT_SOURCE_SHA}\n")
string(SHA256 SOAM_TRANSITION_INVARIANT_IMPLEMENTATION_REVISION_SHA
    "${SOAM_TRANSITION_INVARIANT_IMPLEMENTATION_MANIFEST}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/transition_invariant_implementation_revision.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/transition_invariant_implementation_revision.hpp"
    @ONLY)

target_include_directories(soam_transition_invariant
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/generated)

target_link_libraries(soam_transition_invariant
    PUBLIC
        AdaptiveMesh::soam_runtime
        AdaptiveMesh::soam_transition_request)

if(WIN32)
    target_link_libraries(soam_transition_invariant PRIVATE bcrypt)
endif()

target_compile_features(soam_transition_invariant PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soam_transition_invariant PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(SOAM_WARNINGS_AS_ERRORS)
        target_compile_options(soam_transition_invariant PRIVATE -Werror)
    endif()
endif()

enable_testing()
