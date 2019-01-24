cmake_minimum_required(VERSION 3.6)
project(perspective)

set(CMAKE_CXX_STANDARD 11)

set(OpenCV_DIR /usr/share/OpenCV)
find_package(OpenCV REQUIRED)

include(lib/lib_files.cmake)

add_executable(perspective main.cpp ${LIB_FILES} lib/Line.cpp lib/Line.h lib/VanishingPoint.cpp lib/VanishingPoint.h)
target_link_libraries(perspective ${OpenCV_LIBS})
