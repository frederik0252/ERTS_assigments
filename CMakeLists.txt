cmake_minimum_required(VERSION 3.10)
project(EmbeddedSystemX CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)
add_executable(esx main.cpp State.cpp EmbeddedSystemX.cpp StartupStates.cpp
               OperationalStates.cpp ModeStates.cpp RealTimeLoop.cpp)
target_link_libraries(esx Threads::Threads)
