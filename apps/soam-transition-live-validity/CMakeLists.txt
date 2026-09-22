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
