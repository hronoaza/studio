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

add_library(soam_transition_permission STATIC
    src/production_permission_attestation.cpp)
add_library(
    AdaptiveMesh::soam_transition_permission
    ALIAS soam_transition_permission)

target_include_directories(soam_transition_permission
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

target_link_libraries(soam_transition_permission
    PRIVATE soam_sodium)

target_compile_features(soam_transition_permission PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soam_transition_permission PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(SOAM_WARNINGS_AS_ERRORS)
        target_compile_options(soam_transition_permission PRIVATE -Werror)
    endif()
endif()

enable_testing()

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
