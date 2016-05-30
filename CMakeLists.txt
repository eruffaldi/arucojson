cmake_minimum_required(VERSION 3.4)
include(cmakego) # use cmakego from https://github.com/eruffaldi/cmakego

add_definitions(--std=c++11)
usepackage(json eigen aruco REQUIRED)
usepackage(opencv REQUIRED) # already included by previous

add_executable(aruco2json main.cpp)
target_link_libraries(aruco2json p::aruco p::opencv p::json p::eigen)

add_executable(undistort undistort.cpp)
target_link_libraries(undistort p::opencv)

add_executable(overlay overlay.cpp)
target_link_libraries(overlay p::aruco p::opencv p::json p::eigen)