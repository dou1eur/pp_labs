cmake_minimum_required(VERSION 3.10)

project(Lab3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(MPI REQUIRED)

add_executable(lab_3 multiply.cpp)

target_include_directories(lab_3 PRIVATE ${MPI_INCLUDE_PATH})
target_link_libraries(lab_3 PRIVATE ${MPI_CXX_LIBRARIES} rt)