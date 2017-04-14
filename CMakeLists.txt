cmake_minimum_required(VERSION 3.4)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules")

add_definitions(--std=c++11)
find_package(aruco REQUIRED)
find_package(opencv REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(/usr/local/Cellar/eigen/3.3.2/include/eigen3/)

add_executable(aruco2json main.cpp jsoncpp.cpp)
target_link_libraries(aruco2json ${ARUCO_LIBRARIES} ${OpenCV_LIBS})

add_executable(undistort undistort.cpp jsoncpp.cpp)
target_link_libraries(undistort ${OpenCV_LIBS} )

add_executable(overlay overlay.cpp jsoncpp.cpp)
target_link_libraries(overlay ${ARUCO_LIBRARIES} ${OpenCV_LIBS})