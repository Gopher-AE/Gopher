cmake_minimum_required(VERSION 3.10)
project(Gopher)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${PROJECT_SOURCE_DIR}/include)


set(SOURCES
    src/pattern.cpp
    src/mappings.cpp
    src/schedule.cpp
    src/dag.cpp
    src/mining.cpp
    src/baseline_test.cpp
)


add_executable(baseline_test ${SOURCES})


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif() 