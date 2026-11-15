cmake_minimum_required(VERSION 3.16)
project(erts_assignment_3 LANGUAGES CXX)
find_package(Threads REQUIRED)

add_library(embedded_system_x
    src/State.cpp
    src/EmbeddedSystemX.cpp
    src/StartupStates.cpp
    src/OperationalStates.cpp
    src/RealTimeLoopActiveObject.cpp
)
target_link_libraries(embedded_system_x PUBLIC Threads::Threads)
target_include_directories(embedded_system_x PUBLIC include)
target_compile_features(embedded_system_x PUBLIC cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(embedded_system_x PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(erts_console src/main.cpp)
target_link_libraries(erts_console PRIVATE embedded_system_x)

include(CTest)
if(BUILD_TESTING)
    add_executable(startup_tests tests/startup_tests.cpp)
    target_link_libraries(startup_tests PRIVATE embedded_system_x)
    add_test(NAME startup_behavior COMMAND startup_tests)
    add_executable(operational_tests tests/operational_tests.cpp)
    target_link_libraries(operational_tests PRIVATE embedded_system_x)
    add_test(NAME operational_behavior COMMAND operational_tests)
    set_tests_properties(startup_behavior operational_behavior PROPERTIES TIMEOUT 15)
endif()
