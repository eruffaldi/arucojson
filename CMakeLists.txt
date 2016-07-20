cmake_minimum_required(VERSION 3.4)

add_definitions(--std=c++11)
find_package(cmakego COMPONENTS json eigen aruco opencv REQUIRED) # use cmakego from https://github.com/eruffaldi/cmakego

add_executable(aruco2json main.cpp)
target_link_libraries(aruco2json p::aruco p::opencv p::json p::eigen)

add_executable(undistort undistort.cpp)
target_link_libraries(undistort p::opencv)

add_executable(overlay overlay.cpp)
target_link_libraries(overlay p::aruco p::opencv p::json p::eigen)