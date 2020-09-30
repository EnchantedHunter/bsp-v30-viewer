cmake_minimum_required(VERSION 2.6)
  
project(BSPVIEWER)

option(BUILD_OGL43 "Bild OpenGL 3.3" OFF)
mark_as_advanced(BUILD_OGL43)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(glxw)

include_directories(glfw/include)
include_directories(glm)
include_directories(${CMAKE_BINARY_DIR}/glxw/include)
 
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra")
SET(LIBRARIES glfw glxw ${GLFW_LIBRARIES} ${GLXW_LIBRARY} ${OPENGL_LIBRARY} ${CMAKE_DL_LIBS})

link_directories (${OPENGLEXAMPLES_BINARY_DIR}/bin)
 
add_executable (bspv src/main.cpp src/bsp.cpp src/utils.cpp src/camera.cpp src/container.cpp)
target_link_libraries(bspv ${LIBRARIES} )