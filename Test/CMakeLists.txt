cmake_minimum_required(VERSION 2.6)



project(Test)

file(GLOB_RECURSE CPP_FILES *.cpp gtest_main.cc)

add_executable(${PROJECT_NAME} ${CPP_FILES})

include_directories(${SlispLib_INCLUDE_DIRS})
include_directories(../Vendor/googletest-release-1.7.0/include)
target_link_libraries(${PROJECT_NAME} SlispLib gtest)

file(GLOB TEST_FILES "Files/*")
file(COPY ${TEST_FILES} DESTINATION ${PROJECT_BINARY_DIR})

