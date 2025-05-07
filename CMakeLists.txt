cmake_minimum_required(VERSION 3.10)
project(WindowOnly)

set(CMAKE_CXX_STANDARD 17)

add_executable(window src/main.cpp)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(
    /usr/include/GL/
    /usr/include/glad/
    /usr/include/GLFW/
    /usr/include/openpgl/
    
)

target_link_libraries(window
    ${GLEW_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${OPENGL_gl_LIBRARY}
)
