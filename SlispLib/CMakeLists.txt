cmake_minimum_required(VERSION 2.6)

project(SlispLib)

file(GLOB_RECURSE CPP_FILES *.cpp)
add_library(${PROJECT_NAME} STATIC ${CPP_FILES})

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

