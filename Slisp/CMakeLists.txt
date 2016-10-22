cmake_minimum_required(VERSION 2.6)

project(Slisp)

include_directories(${SlispLib_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} Slisp.cpp)
target_link_libraries(${PROJECT_NAME} SlispLib)

