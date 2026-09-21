cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
project(SOAMTransitionEligibility VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SOAM_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)

add_library(soam_transition_eligibility INTERFACE)
target_include_directories(soam_transition_eligibility INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(soam_transition_eligibility INTERFACE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(soam_transition_eligibility INTERFACE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    if(SOAM_WARNINGS_AS_ERRORS)
        target_compile_options(soam_transition_eligibility INTERFACE -Werror)
    endif()
endif()

enable_testing()
add_executable(eligibility_tests tests/eligibility_tests.cpp)
target_link_libraries(eligibility_tests PRIVATE soam_transition_eligibility)
add_test(NAME run_eligibility_tests COMMAND eligibility_tests)
