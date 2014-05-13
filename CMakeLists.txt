project(ClothSimulation)
cmake_minimum_required(VERSION 2.8)

Set(CMAKE_BUILD_TYPE Release)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(OpenGL)
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
#add_definitions(${Qt5Widgets_DEFINITIONS})
#add_definitions(${Qt5Opengl_DEFINITIONS})
#include_directories(${Qt5Widgets_INCLUDES})
#include_directories(${Qt5Opengl_INCLUDES})


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -std=c++11")

SET(OPENCL_ROOT "C:/Program Files (x86)/AMD APP SDK/2.9")
INCLUDE_DIRECTORIES("${OPENCL_ROOT}/include")
SET(OPENCL_LIBRARIES "C:/Program Files (x86)/AMD APP SDK/2.9/lib/x86/OpenCL.lib")
SET(OPENCL_DLL "C:/Program Files (x86)/AMD APP SDK/2.9/bin/x86/OpenCL.dll")

QT5_WRAP_UI(UIS_HDRS
    mainwindow.ui
)
aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST} ${UIS_HDRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${OPENGL_INCLUDE_DIR})
qt5_use_modules(${PROJECT_NAME} Widgets OpenGL)
MESSAGE(STATUS ${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OPENCL_LIBRARIES} ${OPENCL_DLL} ${Qt5Widgets_LIBRARIES} ${Qt5Opengl_LIBRARIES} ${OPENGL_LIBRARIES})
