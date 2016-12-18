cmake_minimum_required(VERSION 2.6)

if(MSVC)
  # Multithreaded Debug DLL - not the default for gtest
  # TODO: Release vs Debug configurations
  add_definitions("/MDd")
else()
  add_definitions("--std=c++11")
endif()

add_subdirectory(SlispLib)
add_subdirectory(Slisp)
add_subdirectory(Vendor/googletest-release-1.7.0)
add_subdirectory(Test)

enable_testing()
add_test(TestSlispLib Test/Test)
add_test(NAME TestSlisp
         COMMAND Slisp tests/RunTests.slisp
         WORKING_DIRECTORY ${Slisp_BINARY_DIR})
