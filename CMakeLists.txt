cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11")

add_subdirectory(SlispLib)
add_subdirectory(Slisp)
add_subdirectory(Vendor/googletest-release-1.7.0)
add_subdirectory(Test)

enable_testing()
add_test(unittests Test/Test)

