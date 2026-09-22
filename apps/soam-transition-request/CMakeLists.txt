cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
project(SOAMTransitionRequest VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOAM_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../soam-runtime"
    "${CMAKE_CURRENT_BINARY_DIR}/soam-runtime")
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../soam-transition"
    "${CMAKE_CURRENT_BINARY_DIR}/soam-transition")

add_library(soam_transition_request STATIC
    src/production_transition_request.cpp)
add_library(AdaptiveMesh::soam_transition_request ALIAS soam_transition_request)

file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/include/production_transition_request.hpp"
    SOAM_TRANSITION_REQUEST_PUBLIC_SHA)
file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/production_transition_request_internal.hpp"
    SOAM_TRANSITION_REQUEST_INTERNAL_SHA)
file(SHA256
    "${CMAKE_CURRENT_SOURCE_DIR}/src/production_transition_request.cpp"
    SOAM_TRANSITION_REQUEST_SOURCE_SHA)
set(SOAM_TRANSITION_REQUEST_IMPLEMENTATION_MANIFEST
    "SOAM:TRANSITION-REQUEST:IMPLEMENTATION-MANIFEST:V1\npublic=${SOAM_TRANSITION_REQUEST_PUBLIC_SHA}\ninternal=${SOAM_TRANSITION_REQUEST_INTERNAL_SHA}\nsource=${SOAM_TRANSITION_REQUEST_SOURCE_SHA}\n")
string(SHA256 SOAM_TRANSITION_REQUEST_IMPLEMENTATION_REVISION_SHA
    "${SOAM_TRANSITION_REQUEST_IMPLEMENTATION_MANIFEST}")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/detail/transition_request_implementation_revision.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/transition_request_implementation_revision.hpp"
    @ONLY)

target_include_directories(soam_transition_request
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/generated)

target_link_libraries(soam_transition_request
    PUBLIC
        AdaptiveMesh::soam_runtime
        soam_transition_eligibility)

if(WIN32)
    target_link_libraries(soam_transition_request PRIVATE bcrypt)
endif()

target_compile_features(soam_transition_request PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soam_transition_request PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(SOAM_WARNINGS_AS_ERRORS)
        target_compile_options(soam_transition_request PRIVATE -Werror)
    endif()
endif()

enable_testing()

add_executable(transition_request_tests tests/transition_request_tests.cpp)
target_link_libraries(transition_request_tests PRIVATE soam_transition_request)
target_include_directories(transition_request_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME run_transition_request_tests COMMAND transition_request_tests)
