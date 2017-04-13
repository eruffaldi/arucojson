cmake_minimum_required(VERSION 3.4)

add_definitions(--std=c++11)
find_package(cmakego COMPONENTS eigen aruco opencv2 REQUIRED) # use cmakego from https://github.com/eruffaldi/cmakego

add_executable(aruco2json main.cpp jsoncpp.cpp)
target_link_libraries(aruco2json p::aruco p::opencv2  p::eigen)

add_executable(undistort undistort.cpp jsoncpp.cpp)
target_link_libraries(undistort p::opencv2)

add_executable(overlay overlay.cpp jsoncpp.cpp)
target_link_libraries(overlay p::aruco p::opencv2  p::eigen)