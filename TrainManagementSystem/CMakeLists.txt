cmake_minimum_required(VERSION 3.16)
project(TrainManagementSystem VERSION 1.0.0 LANGUAGES CXX C)

# Modern C++ standard requirement
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warning flags for strict quality standards
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Dependencies
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/Managers
    ${SQLite3_INCLUDE_DIRS}
)

# Core Library Sources
set(TMS_CORE_SOURCES
    src/User.cpp
    src/Train.cpp
    src/Station.cpp
    src/Booking.cpp
    src/Payment.cpp
    src/Managers/DatabaseManager.cpp
    src/Managers/StationManager.cpp
)

# Core Library Target
add_library(tms_core STATIC ${TMS_CORE_SOURCES})
target_include_directories(tms_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/Managers
)
target_link_libraries(tms_core PUBLIC
    ${SQLite3_LIBRARIES}
    Threads::Threads
)

# Main Application Executable
add_executable(tms_app src/main.cpp)
target_link_libraries(tms_app PRIVATE tms_core)

# Testing configuration
enable_testing()

# Try finding system GTest or fallback to bundled/FetchContent
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# Test Executable Target
set(TMS_TEST_SOURCES
    tests/test_main.cpp
    tests/test_phase1.cpp
)

add_executable(tms_tests ${TMS_TEST_SOURCES})
target_link_libraries(tms_tests PRIVATE
    tms_core
    GTest::gtest
    GTest::gtest_main
    Threads::Threads
)

# Register tests with CTest
add_test(NAME Phase1UnitTests COMMAND tms_tests)
